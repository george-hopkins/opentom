/*
 * gpsdash - GPS dashboard for OpenTom / Nano-X (TomTom XL LIVE, 480x272)
 *
 * Reads NMEA text from the GPS pipe (default /var/run/gpspipe, filled by
 * glgps) and shows speed, heading, altitude, position, satellites and a
 * per-satellite signal bar chart.  Touch buttons: LOG, UNITS, EXIT.
 * The buttons are kept in the middle of the screen on purpose.
 *
 * usage: gpsdash [pipe]
 *
 * Built by the OpenTom build (make -C applications/src/gpsdash install, after
 * sourcing get_cross_env.sh); started by src/opentom_skel/bin/gpsdash.sh.
 * Manual build for the device:
 *   arm-linux-gcc -O2 -o gpsdash gpsdash.c -I<sysroot>/usr/include/microwin \
 *       -L<sysroot>/usr/lib -lnano-X
 * Host self-test of the NMEA parser (no Nano-X needed):
 *   gcc -DGPSDASH_TEST -o gpsdash_test gpsdash.c && ./gpsdash_test < sample.nmea
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#ifndef GPSDASH_TEST
#include <microwin/nano-X.h>
#endif

#define MAXSATS 24
#define LOGFILE "/mnt/sdcard/opentom/logs/gps-track.csv"

struct sat {
	int prn, snr, elev;
};

struct gps {
	int valid;              /* RMC status A */
	int quality;            /* GGA fix quality, 0 = none */
	int fixmode;            /* GSA: 1 none, 2 = 2D, 3 = 3D */
	int nused, nview;
	int have_pos, have_course, have_alt;
	double lat, lon, knots, course, alt_m, hdop;
	char utc[16];           /* "HH:MM:SS" */
	char date[16];          /* "DD/MM/YY" */
	struct sat sats[MAXSATS];
	int nsats;
	struct sat tmp[MAXSATS];
	int ntmp;
	int used[12];           /* PRNs used in the fix (GSA) */
	unsigned long rmc_count;
};

/* ------------------------------------------------------------------ */
/* NMEA parsing                                                        */
/* ------------------------------------------------------------------ */

static int split(char *s, char **f, int max)
{
	int n = 0;
	char *p = s, *star = strchr(s, '*');

	if (star)
		*star = 0;
	f[n++] = p;
	while (*p && n < max) {
		if (*p == ',') {
			*p = 0;
			f[n++] = p + 1;
		}
		p++;
	}
	return n;
}

static double nmea_deg(const char *v, const char *hemi)
{
	double x, d, m;

	if (!v[0])
		return 0;
	x = atof(v);
	d = (int)(x / 100.0);
	m = x - d * 100.0;
	d = d + m / 60.0;
	if (hemi[0] == 'S' || hemi[0] == 'W')
		d = -d;
	return d;
}

/*
 * GPS week rollover: older receivers (this one included) count weeks in 10
 * bits, which wrapped in 2019, so they report dates exactly 1024 weeks
 * (7168 days) too early.  Any date before 2024 is shifted forward by 7168
 * days; the time of day is unaffected.
 */
static long days_from_civil(int y, int m, int d)
{
	long era;
	unsigned yoe, doy, doe;

	y -= (m <= 2);
	era = (y >= 0 ? y : y - 399) / 400;
	yoe = (unsigned)(y - era * 400);
	doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
	doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + (long)doe - 719468;
}

static void civil_from_days(long z, int *y, int *m, int *d)
{
	long era, yy;
	unsigned doe, yoe, doy, mp;

	z += 719468;
	era = (z >= 0 ? z : z - 146096) / 146097;
	doe = (unsigned)(z - era * 146097);
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	yy = (long)yoe + era * 400;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	mp = (5 * doy + 2) / 153;
	*d = (int)(doy - (153 * mp + 2) / 5 + 1);
	*m = (int)(mp < 10 ? mp + 3 : mp - 9);
	*y = (int)(yy + (*m <= 2));
}

static void fix_date(const char *ddmmyy, char *out, size_t outsz)
{
	char t[3];
	int d, m, y;

	t[2] = 0;
	t[0] = ddmmyy[0]; t[1] = ddmmyy[1]; d = atoi(t);
	t[0] = ddmmyy[2]; t[1] = ddmmyy[3]; m = atoi(t);
	t[0] = ddmmyy[4]; t[1] = ddmmyy[5]; y = 2000 + atoi(t);
	if (y < 2024 && m >= 1 && m <= 12 && d >= 1 && d <= 31)
		civil_from_days(days_from_civil(y, m, d) + 7168, &y, &m, &d);
	snprintf(out, outsz, "%02d/%02d/%02d", d, m, y % 100);
}

static void parse_line(struct gps *g, char *line)
{
	char *f[32];
	int n, k, i;
	const char *t;

	while (*line == '\r' || *line == '\n' || *line == ' ')
		line++;
	if (line[0] != '$' || strlen(line) < 7)
		return;
	n = split(line, f, 32);
	if (strlen(f[0]) != 6)
		return;
	t = f[0] + 3;

	if (!strcmp(t, "RMC") && n >= 10) {
		if (strlen(f[1]) >= 6) {
			snprintf(g->utc, sizeof g->utc, "%.2s:%.2s:%.2s", f[1], f[1] + 2, f[1] + 4);
		}
		if (strlen(f[9]) >= 6) {
			fix_date(f[9], g->date, sizeof g->date);
		}
		g->valid = (f[2][0] == 'A');
		if (g->valid) {
			g->have_pos = (f[3][0] && f[5][0]);
			if (g->have_pos) {
				g->lat = nmea_deg(f[3], f[4]);
				g->lon = nmea_deg(f[5], f[6]);
			}
			g->knots = f[7][0] ? atof(f[7]) : 0;
			g->have_course = (f[8][0] != 0);
			if (g->have_course)
				g->course = atof(f[8]);
		} else {
			g->have_pos = 0;
			g->have_course = 0;
			g->knots = 0;
		}
		g->rmc_count++;
	} else if (!strcmp(t, "GGA") && n >= 10) {
		g->quality = atoi(f[6]);
		g->nused = atoi(f[7]);
		if (f[8][0])
			g->hdop = atof(f[8]);
		g->have_alt = (f[9][0] && g->quality > 0);
		if (g->have_alt)
			g->alt_m = atof(f[9]);
	} else if (!strcmp(t, "GSA") && n >= 17) {
		g->fixmode = atoi(f[2]);
		for (i = 0; i < 12; i++)
			g->used[i] = atoi(f[3 + i]);
	} else if (!strcmp(t, "GSV") && n >= 4) {
		int total = atoi(f[1]), num = atoi(f[2]);

		if (num == 1)
			g->ntmp = 0;
		for (k = 4; k + 2 < n; k += 4) {
			if (g->ntmp < MAXSATS) {
				g->tmp[g->ntmp].prn = atoi(f[k]);
				g->tmp[g->ntmp].elev = atoi(f[k + 1]);
				g->tmp[g->ntmp].snr = (k + 3 < n && f[k + 3][0]) ? atoi(f[k + 3]) : 0;
				g->ntmp++;
			}
		}
		if (num == total) {
			memcpy(g->sats, g->tmp, sizeof g->sats);
			g->nsats = g->ntmp;
			g->nview = atoi(f[3]);
		}
	}
}

static char lbuf[512];
static int llen;

static void feed(struct gps *g, const char *buf, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (buf[i] == '\n') {
			lbuf[llen] = 0;
			parse_line(g, lbuf);
			llen = 0;
		} else if (llen < (int)sizeof(lbuf) - 1) {
			lbuf[llen++] = buf[i];
		}
	}
}

static const char *compass(double d)
{
	static const char *nm[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
	int i = (int)((d + 22.5) / 45.0) % 8;

	return nm[i < 0 ? 0 : i];
}

static double absd(double x)
{
	return x < 0 ? -x : x;
}

/*
 * Rough charge level from the resting voltage of a single lithium-ion cell
 * (typical curve).  Accurate to about 10 points; reads a little low while
 * the screen and GPS are drawing power, and is meaningless while charging.
 */
static int batt_percent(int mv)
{
	static const struct { int mv, pct; } tab[] = {
		{ 4200, 100 }, { 4150, 95 }, { 4110, 90 }, { 4080, 85 }, { 4020, 80 },
		{ 3980, 75 }, { 3950, 70 }, { 3910, 65 }, { 3870, 60 }, { 3850, 55 },
		{ 3840, 50 }, { 3820, 45 }, { 3800, 40 }, { 3790, 35 }, { 3770, 30 },
		{ 3750, 25 }, { 3730, 20 }, { 3710, 15 }, { 3690, 10 }, { 3610, 5 },
		{ 3270, 0 }
	};
	int i, n = (int)(sizeof tab / sizeof tab[0]);

	if (mv >= tab[0].mv)
		return 100;
	if (mv <= tab[n - 1].mv)
		return 0;
	for (i = 1; i < n; i++) {
		if (mv >= tab[i].mv)
			return tab[i].pct + (mv - tab[i].mv) * (tab[i - 1].pct - tab[i].pct) / (tab[i - 1].mv - tab[i].mv);
	}
	return 0;
}

/* ------------------------------------------------------------------ */
#ifdef GPSDASH_TEST
int main(void)
{
	struct gps g;
	char buf[256];
	int i;

	memset(&g, 0, sizeof g);
	while (fgets(buf, sizeof buf, stdin))
		feed(&g, buf, strlen(buf));
	printf("valid=%d quality=%d fixmode=%d used=%d view=%d\n", g.valid, g.quality, g.fixmode, g.nused, g.nview);
	printf("pos=%d lat=%.5f lon=%.5f  course=%d:%.1f (%s)  knots=%.1f kmh=%.1f mph=%.1f\n",
	       g.have_pos, g.lat, g.lon, g.have_course, g.course, compass(g.course),
	       g.knots, g.knots * 1.852, g.knots * 1.15078);
	printf("alt=%d %.1f m  hdop=%.1f  utc=%s date=%s rmc=%lu\n", g.have_alt, g.alt_m, g.hdop, g.utc, g.date, g.rmc_count);
	printf("sats(%d):", g.nsats);
	for (i = 0; i < g.nsats; i++)
		printf(" %d/%d", g.sats[i].prn, g.sats[i].snr);
	printf("\nabs(-3.5)=%.1f\n", absd(-3.5));
	printf("batt: 4200=%d 4132=%d 4000=%d 3800=%d 3700=%d 3500=%d 4300=%d 3000=%d\n",
	       batt_percent(4200), batt_percent(4132), batt_percent(4000), batt_percent(3800),
	       batt_percent(3700), batt_percent(3500), batt_percent(4300), batt_percent(3000));
	return 0;
}
#else
/* ------------------------------------------------------------------ */
/* UI                                                                  */
/* ------------------------------------------------------------------ */

#define W 480
#define H 272

static GR_WINDOW_ID win;
static GR_DRAW_ID pix;
static GR_GC_ID gc;

struct rect {
	int x, y, w, h;
};

static const struct rect btn_log = { 80, 156, 90, 26 };
static const struct rect btn_units = { 195, 156, 90, 26 };
static const struct rect btn_exit = { 310, 156, 90, 26 };

static struct gps g;
static int imperial = 1;
static int logging;
static FILE *logf;
static long logpts;
static unsigned long last_logged_rmc;
static int pfd = -1;
static time_t last_rx;
static volatile int quit;

/* Same layout as BATTERY_STATUS in <barcelona/Barc_Battery.h>:
 * UINT16 voltage (mV), UINT16 charge current (mA), UINT8 charge status
 * (0 = no external power, 1 = charge complete, 2 = charging). */
struct batt {
	unsigned short mv;
	unsigned short ma;
	unsigned char st;
};
/* 6 bytes on a modern ABI, 8 on the old ARM ABI (struct padded to a word);
 * the driver was built the same way, and the size is part of the ioctl code,
 * so it is always computed from sizeof() rather than hardcoded. */
typedef char batt_size_check[(sizeof(struct batt) == 6 || sizeof(struct batt) == 8) ? 1 : -1];
#ifndef IOR_BATTERY_STATUS
#ifdef _IOR
#define IOR_BATTERY_STATUS _IOR('B', 3, struct batt)
#else
#define IOR_BATTERY_STATUS ((2UL << 30) | ((unsigned long)sizeof(struct batt) << 16) | ('B' << 8) | 3UL)
#endif
#endif

static int bat_fd = -1;
static int bat_ok;
static struct batt bat;
static time_t bat_last;

static void poll_battery(void)
{
	time_t now = time(NULL);

	if (bat_ok && now - bat_last < 5)
		return;
	bat_last = now;
	if (bat_fd < 0) {
		bat_fd = open("/dev/battery", O_RDWR);
		if (bat_fd < 0)
			return;
	}
	bat_ok = (ioctl(bat_fd, IOR_BATTERY_STATUS, &bat) == 0);
}

#define COL_BG    GR_RGB(8, 10, 14)
#define COL_BAR   GR_RGB(30, 34, 42)
#define COL_TEXT  GR_RGB(215, 220, 228)
#define COL_DIM   GR_RGB(120, 128, 140)
#define COL_GHOST GR_RGB(26, 30, 36)
#define COL_GREEN GR_RGB(60, 230, 110)
#define COL_AMBER GR_RGB(255, 190, 40)
#define COL_RED   GR_RGB(255, 80, 70)
#define COL_CYAN  GR_RGB(90, 210, 255)

static void fill(int x, int y, int w, int h, GR_COLOR c)
{
	if (w <= 0 || h <= 0)
		return;
	GrSetGCForeground(gc, c);
	GrFillRect(pix, gc, x, y, w, h);
}

static void text(int x, int y, GR_COLOR c, const char *s)
{
	GrSetGCForeground(gc, c);
	GrText(pix, gc, x, y, (void *)s, strlen(s), GR_TFASCII | GR_TFTOP);
}

/* faux bold: draw twice, one pixel apart */
static void btext(int x, int y, GR_COLOR c, const char *s)
{
	text(x, y, c, s);
	text(x + 1, y, c, s);
}

static int text_width(const char *s)
{
	GR_SIZE w, h, b;

	GrGetGCTextSize(gc, (void *)s, strlen(s), GR_TFASCII, &w, &h, &b);
	return w;
}

/* 7-segment masks: a=1 b=2 c=4 d=8 e=16 f=32 g=64 */
static const unsigned char segmask[10] = { 63, 6, 91, 79, 102, 109, 125, 7, 127, 111 };

static void draw_cell(int x, int y, int w, int h, int t, int mask, GR_COLOR on)
{
	int half = h / 2, uh = half - t / 2 - t, lh = h - half - (3 * t) / 2;
	int s;
	struct { int x, y, w, h; } seg[7];

	seg[0].x = x + t;      seg[0].y = y;                 seg[0].w = w - 2 * t; seg[0].h = t;  /* a */
	seg[1].x = x + w - t;  seg[1].y = y + t;             seg[1].w = t;         seg[1].h = uh; /* b */
	seg[2].x = x + w - t;  seg[2].y = y + half + t / 2;  seg[2].w = t;         seg[2].h = lh; /* c */
	seg[3].x = x + t;      seg[3].y = y + h - t;         seg[3].w = w - 2 * t; seg[3].h = t;  /* d */
	seg[4].x = x;          seg[4].y = y + half + t / 2;  seg[4].w = t;         seg[4].h = lh; /* e */
	seg[5].x = x;          seg[5].y = y + t;             seg[5].w = t;         seg[5].h = uh; /* f */
	seg[6].x = x + t;      seg[6].y = y + half - t / 2;  seg[6].w = w - 2 * t; seg[6].h = t;  /* g */

	for (s = 0; s < 7; s++)
		fill(seg[s].x, seg[s].y, seg[s].w, seg[s].h, (mask & (1 << s)) ? on : COL_GHOST);
}

/* returns x after the last character */
static int draw_num(int x, int y, int w, int h, int t, int gap, const char *s, GR_COLOR on)
{
	for (; *s; s++) {
		if (*s >= '0' && *s <= '9') {
			draw_cell(x, y, w, h, t, segmask[*s - '0'], on);
			x += w + gap;
		} else if (*s == '-') {
			draw_cell(x, y, w, h, t, 64, on);
			x += w + gap;
		} else if (*s == '.') {
			fill(x, y + h - t, t, t, on);
			x += t + gap;
		} else {
			draw_cell(x, y, w, h, t, 0, on);
			x += w + gap;
		}
	}
	return x;
}

static void button(const struct rect *r, const char *label, GR_COLOR edge)
{
	int tw = text_width(label);

	fill(r->x, r->y, r->w, r->h, edge);
	fill(r->x + 2, r->y + 2, r->w - 4, r->h - 4, COL_BAR);
	btext(r->x + (r->w - tw) / 2, r->y + 8, COL_TEXT, label);
}

static int inside(const struct rect *r, int x, int y)
{
	return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

static int is_used(int prn)
{
	int i;

	for (i = 0; i < 12; i++)
		if (g.used[i] == prn && prn)
			return 1;
	return 0;
}

static void redraw(void)
{
	char b[64];
	int i, x, hx, now_ok, fix;
	GR_COLOR sc;
	const char *st;

	now_ok = (time(NULL) - last_rx) <= 4;
	fix = now_ok && g.valid && g.quality > 0;

	fill(0, 0, W, H, COL_BG);

	/* header */
	fill(0, 0, W, 22, COL_BAR);
	if (!now_ok) {
		st = "NO DATA";
		sc = COL_RED;
	} else if (fix) {
		st = (g.fixmode == 3) ? "3D FIX" : (g.fixmode == 2) ? "2D FIX" : "FIX";
		sc = COL_GREEN;
	} else {
		st = "SEARCHING";
		sc = COL_AMBER;
	}
	hx = 8;
	btext(hx, 5, sc, st);
	hx += text_width(st) + 16;
	snprintf(b, sizeof b, "SATS %d/%d", g.nused, g.nview);
	btext(hx, 5, COL_TEXT, b);
	hx += text_width(b) + 16;
	if (bat_ok) {
		GR_COLOR bc = COL_TEXT;

		if (bat.st == 2) {
			snprintf(b, sizeof b, "CHG %d.%02dV", bat.mv / 1000, (bat.mv % 1000) / 10);
			bc = COL_CYAN;
		} else if (bat.st == 1) {
			snprintf(b, sizeof b, "BAT FULL");
			bc = COL_GREEN;
		} else {
			int pct = batt_percent(bat.mv);

			snprintf(b, sizeof b, "BAT %d%%", pct);
			bc = pct >= 50 ? COL_GREEN : pct >= 20 ? COL_AMBER : COL_RED;
		}
		btext(hx, 5, bc, b);
		hx += text_width(b) + 16;
	}
	snprintf(b, sizeof b, "%s UTC  %s", g.utc[0] ? g.utc : "--:--:--", g.date[0] ? g.date : "--/--/--");
	text(W - 8 - text_width(b), 5, COL_DIM, b);

	/* speed */
	text(10, 27, COL_DIM, "SPEED");
	if (fix)
		snprintf(b, sizeof b, "%3d", (int)(g.knots * (imperial ? 1.15078 : 1.852) + 0.5));
	else
		snprintf(b, sizeof b, "---");
	x = draw_num(10, 40, 44, 76, 9, 8, b, COL_CYAN);
	btext(x + 6, 100, COL_TEXT, imperial ? "MPH" : "KM/H");

	/* heading */
	text(250, 27, COL_DIM, "HEADING");
	if (fix && g.have_course) {
		snprintf(b, sizeof b, "%3d", ((int)(g.course + 0.5)) % 360);
		x = draw_num(250, 40, 22, 34, 5, 5, b, COL_GREEN);
		btext(x + 8, 48, COL_TEXT, compass(g.course));
	} else {
		x = draw_num(250, 40, 22, 34, 5, 5, "---", COL_GREEN);
	}

	/* altitude */
	text(250, 80, COL_DIM, "ALTITUDE");
	if (g.have_alt && now_ok) {
		snprintf(b, sizeof b, "%5d", (int)((imperial ? g.alt_m * 3.28084 : g.alt_m) + (g.alt_m < 0 ? -0.5 : 0.5)));
	} else {
		snprintf(b, sizeof b, "-----");
	}
	x = draw_num(250, 92, 17, 26, 4, 4, b, COL_AMBER);
	btext(x + 6, 100, COL_TEXT, imperial ? "FT" : "M");

	/* position */
	text(10, 128, COL_DIM, "LAT");
	text(250, 128, COL_DIM, "LON");
	if (fix && g.have_pos) {
		snprintf(b, sizeof b, "%.5f", absd(g.lat));
		x = draw_num(40, 125, 12, 22, 3, 3, b, COL_TEXT);
		btext(x + 4, 130, COL_TEXT, g.lat < 0 ? "S" : "N");
		snprintf(b, sizeof b, "%.5f", absd(g.lon));
		x = draw_num(282, 125, 12, 22, 3, 3, b, COL_TEXT);
		btext(x + 4, 130, COL_TEXT, g.lon < 0 ? "W" : "E");
	} else {
		draw_num(40, 125, 12, 22, 3, 3, "--.-----", COL_DIM);
		draw_num(282, 125, 12, 22, 3, 3, "--.-----", COL_DIM);
	}

	/* buttons */
	button(&btn_log, logging ? "LOG ON" : "LOG OFF", logging ? COL_GREEN : COL_DIM);
	button(&btn_units, imperial ? "UNITS: MPH" : "UNITS: KM/H", COL_DIM);
	button(&btn_exit, "EXIT", COL_RED);
	if (logging) {
		snprintf(b, sizeof b, "REC %ld", logpts);
		btext(hx, 5, COL_GREEN, b);
	}

	/* satellite signal bars */
	text(10, 190, COL_DIM, "SIGNAL (dB-Hz)   bright = used in fix");
	fill(8, 252, W - 16, 1, COL_DIM);
	for (i = 0; i < g.nsats && i < 12; i++) {
		int bh = g.sats[i].snr;
		int bx = 12 + i * 38;
		GR_COLOR c;
		int used = is_used(g.sats[i].prn);

		if (bh > 50)
			bh = 50;
		if (g.sats[i].snr >= 30)
			c = used ? COL_GREEN : GR_RGB(30, 110, 60);
		else if (g.sats[i].snr >= 20)
			c = used ? COL_AMBER : GR_RGB(120, 90, 20);
		else if (g.sats[i].snr > 0)
			c = used ? COL_RED : GR_RGB(110, 40, 36);
		else
			c = COL_DIM;
		fill(bx, 252 - (bh ? bh : 2), 28, bh ? bh : 2, c);
		if (g.sats[i].snr > 0) {
			snprintf(b, sizeof b, "%d", g.sats[i].snr);
			text(bx + 14 - text_width(b) / 2, 252 - bh - 12, COL_TEXT, b);
		}
		snprintf(b, sizeof b, "%d", g.sats[i].prn);
		text(bx + 14 - text_width(b) / 2, 256, COL_DIM, b);
	}
	if (!g.nsats)
		text(12, 225, COL_DIM, "waiting for satellite data...");

	GrCopyArea(win, gc, 0, 0, W, H, pix, 0, 0, MWROP_COPY);
}

static void poll_pipe(const char *path)
{
	char buf[512];
	int n, guard;

	if (pfd < 0) {
		/* O_RDWR: we also hold a write end, so the pipe never reports EOF
		 * and glgps never gets SIGPIPE if it opens/writes while we poll */
		pfd = open(path, O_RDWR | O_NONBLOCK);
		if (pfd < 0)
			return;
	}
	for (guard = 0; guard < 20; guard++) {
		n = read(pfd, buf, sizeof buf);
		if (n > 0) {
			last_rx = time(NULL);
			feed(&g, buf, n);
			continue;
		}
		if (n == 0) { /* no writer right now: reopen on the next tick */
			close(pfd);
			pfd = -1;
		}
		break;
	}
}

static void log_point(void)
{
	if (!logging || !logf)
		return;
	if (g.rmc_count == last_logged_rmc)
		return;
	last_logged_rmc = g.rmc_count;
	if (!g.valid || !g.have_pos)
		return;
	fprintf(logf, "%s,%s,%.6f,%.6f,%.1f,%.1f,%.1f,%d\n", g.date, g.utc, g.lat, g.lon,
		g.knots * 1.852, g.have_course ? g.course : 0.0, g.have_alt ? g.alt_m : 0.0, g.nused);
	fflush(logf);
	logpts++;
	if (logpts % 10 == 0)
		fsync(fileno(logf));
}

static void toggle_log(void)
{
	if (logging) {
		logging = 0;
		if (logf) {
			fclose(logf);
			logf = NULL;
		}
		return;
	}
	logf = fopen(LOGFILE, "a");
	if (!logf)
		return;
	if (ftell(logf) <= 0)
		fprintf(logf, "date,utc,lat,lon,speed_kmh,heading,alt_m,sats\n");
	logpts = 0;
	last_logged_rmc = g.rmc_count;
	logging = 1;
}

static void on_term(int sig)
{
	(void)sig;
	quit = 1;
}

int main(int argc, char **argv)
{
	const char *pipepath = (argc > 1) ? argv[1] : "/var/run/gpspipe";
	GR_SCREEN_INFO si;
	GR_EVENT ev;
	time_t last_tap = 0;
	int tap_ms_guard = 0;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, on_term);
	signal(SIGINT, on_term);

	if (GrOpen() < 0) {
		fprintf(stderr, "gpsdash: cannot connect to Nano-X server\n");
		return 1;
	}
	GrGetScreenInfo(&si);
	win = GrNewWindowEx(GR_WM_PROPS_NODECORATE, "gpsdash", GR_ROOT_WINDOW_ID, 0, 0, si.cols, si.rows, GR_RGB(0, 0, 0));
	GrSelectEvents(win, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_TIMEOUT);
	GrMapWindow(win);
	gc = GrNewGC();
	GrSetGCUseBackground(gc, GR_FALSE);
	pix = GrNewPixmap(W, H, NULL);

	memset(&g, 0, sizeof g);
	redraw();

	while (!quit) {
		GrGetNextEventTimeout(&ev, 500);
		switch (ev.type) {
		case GR_EVENT_TYPE_TIMEOUT:
			poll_battery();
			poll_pipe(pipepath);
			log_point();
			redraw();
			break;
		case GR_EVENT_TYPE_EXPOSURE:
			redraw();
			break;
		case GR_EVENT_TYPE_BUTTON_DOWN:
			/* crude debounce: ignore a second tap within the same second */
			if (time(NULL) == last_tap && tap_ms_guard)
				break;
			last_tap = time(NULL);
			tap_ms_guard = 1;
			if (inside(&btn_exit, ev.button.x, ev.button.y))
				quit = 1;
			else if (inside(&btn_units, ev.button.x, ev.button.y))
				imperial = !imperial;
			else if (inside(&btn_log, ev.button.x, ev.button.y))
				toggle_log();
			redraw();
			break;
		case GR_EVENT_TYPE_CLOSE_REQ:
			quit = 1;
			break;
		}
		if (time(NULL) != last_tap)
			tap_ms_guard = 0;
	}

	if (logf)
		fclose(logf);
	if (pfd >= 0)
		close(pfd);
	if (bat_fd >= 0)
		close(bat_fd);
	GrDestroyWindow(win);
	GrClose();
	return 0;
}
#endif
