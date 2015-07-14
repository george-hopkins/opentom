/*
 * OpenTom nxmenu, by Clément GERARDIN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#define _GNU_SOURCE
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define MWINCLUDECOLORS
#include <microwin/nano-X.h>
#include "icon.c"

#define MAX_ARGS 30

// sert au décalage de '*' (running)
#define BORDER_R 5
// décalage X du texte dans le menu
#define BORDER_X 12
// décalage X du texte dans le menu
#define BORDER_Y 10

#define RUN_TYPE_NORMAL 0
#define RUN_TYPE_ONCE	1
#define RUN_TYPE_ONOFF  2
#define RUN_TYPE_SEPARATOR	4

// Nombre maxi d'item visible dans un menu
#define MAX_VISIBLE_ITEM	15

struct _sMenuWindow;

typedef struct _sMenuItem {
		struct _sMenuWindow *sub_window;	// = NULL si c'est pas un sous menu	
		char *name, *cmd, *args[MAX_ARGS];
		int launched, run_type;	
		struct _sMenuItem *next;
	} sMenuItem, *pMenuItem;

typedef struct _sMenuWindow {
	GR_WINDOW_ID id;
	GR_SIZE x, width, height;
	int extended, nb_items, scroll_to, sel_item;
	pMenuItem items;
	
	struct _sMenuWindow *next;	// pour utilisation sous forme de liste dans le main
} sMenuWindow, *pMenuWindow;
	
typedef struct _sPid {
		pid_t pid;
		pMenuItem item;
		struct _sPid *next;
	} sPid, *pPid;
		
typedef struct _sAppData {
		GR_GC_ID gc;
		int quit, null_fd; 
		pid_t last_terminated;
		int text_h;	
		pMenuWindow win;
		pPid pids;
		int time, best_time[4];
	} sAppData, *pAppData;

pAppData app;

#define  DELAY 250000

static void alam_handler( int sig) {

	//GrRaiseWindow( app->win->id);
}

static void killall() {
	pPid pp = app->pids;
	while ( pp) {
		kill(pp->pid, SIGQUIT);
		kill(pp->pid, SIGTERM);
		pp = pp->next;
	}
}

void *my_malloc( size_t size) {
	void *b = malloc( size);
	if ( !b) {
		fprintf(stderr,"Out of memory !\n");
		exit(1);
	} else
		return b;
}

int remove_pid( pid_t fpid) {
	pPid pp, pp2;
	
	if ( fpid > 0) {
		pp = app->pids;
		if ( pp->pid == -1 ) {
			fprintf(stderr, "Warning: cannot get new PID\n");
			return 0;
		} if ( pp->pid == fpid) {
			app->pids = pp->next;
			free(pp);
		} else {
			while ( pp->next && (pp->next->pid != fpid)) pp = pp->next;
			if ( pp->next) {
				pp2 = pp->next;
				pp->next = pp2->next;
				free(pp2);
			} else {
				fprintf(stderr, "Warning: Couldn't find PID %d in running pids !\n", fpid);
				return 0;
			}
		}
	}
	return 1;
}

static void fils_action (int sig) {
	int status;
	pid_t fpid;
	
	fpid = waitpid( -1, &status, WNOHANG);
	if ( ! remove_pid( fpid))
		app->last_terminated = fpid;
	else
		app->last_terminated = -1;
		
}

int step_elapsed() {
	static struct timeval oldtime;
	
	struct timeval currenttime;
	long     a,b;

	gettimeofday( &currenttime, NULL);
	a = (currenttime.tv_sec - oldtime.tv_sec);
	b = (currenttime.tv_usec - oldtime.tv_usec);
	if ( a || b > DELAY)
	{
		oldtime = currenttime;
		return 1;
	}
	else return 0;
}	

pMenuWindow new_window() {
	pMenuWindow win = my_malloc( sizeof( sMenuWindow));
	memset( win, 0, sizeof( sMenuWindow));
	return win;
}

pMenuItem add_item_to_window( pMenuWindow win, pMenuItem item) {
	pMenuItem *i = &win->items;

	win->nb_items++;
	while (*i) i = &((*i)->next);
	
	(*i)= item;
	return item;
	
}

pMenuItem new_separator_item() {
	pMenuItem pi = my_malloc( sizeof(sMenuItem));
	memset( pi, 0, sizeof(sMenuItem));
	pi->run_type = RUN_TYPE_SEPARATOR;
	return pi;
}

pMenuItem new_submenu_item( char * name) {
	pMenuItem pi = my_malloc( sizeof(sMenuItem));
	memset( pi, 0, sizeof(sMenuItem));
	pi->name = strdup(name);
	pi->sub_window = new_window();
	return pi;
}

pMenuItem new_program_item( char *name, int run_type, char *cmd ) {
	
	char arg[250], *tmp; 
	int nb_args, in_arg, antislash, longueur_arg;
	
	pMenuItem pi = my_malloc( sizeof(sMenuItem));
	pi->name = strdup(name);
	pi->run_type = run_type;
	pi->launched = in_arg = antislash = longueur_arg = 0;
	
	pi->args[0] = pi->cmd = strdup(strtok( cmd, " \t"));
	tmp = strtok( NULL, "\n\r");
	if ( !tmp) {
		pi->args[1] = NULL;
	}
	else {
		nb_args = 1;
		while( *tmp ) {
			if ( ! antislash && (*tmp == '\\')) {
				antislash=1;
			} else if ( in_arg && ((*tmp == ' ') || (*tmp == '\t')) && !antislash) {
				pi->args[nb_args++] = strndup( arg, longueur_arg);
				in_arg = longueur_arg = 0;
			} else 
				if ( !in_arg && ((*tmp == ' ') || ( *tmp == '\t')) && !antislash); // ignore multiple space
				else {
					arg[longueur_arg++] = *tmp;
					in_arg = 1;
					antislash=0;
				}
			tmp++;
		}
		if ( in_arg ) pi->args[nb_args++] = strndup( arg, longueur_arg);
		pi->args[nb_args] = NULL;
	}
	
	return pi;
}

void run_item( pMenuItem m ) {

	pid_t new_pid;
	pPid pp;
	
	if ( m->run_type != RUN_TYPE_NORMAL ) {
		for ( pp = app->pids; pp && (pp->item != m); pp=pp->next);
		if ( pp ) {
			if (m->run_type == RUN_TYPE_ONCE) return;
			else {
				kill( pp->pid, SIGQUIT);
				kill( pp->pid, SIGTERM);
				return;
			}
		}
	}
	
	pp = my_malloc(sizeof(sPid));	
	pp->item = m;
	pp->pid = -1;
	pp->next = app->pids;		
	app->pids = pp;
	
	if ( ! ( new_pid = fork())) {
		// printf("Starting [%s][%s]\n", m->name, m->cmd);
		close(0); dup(app->null_fd);
		close(1); dup(app->null_fd);
		execvp( m->cmd, m->args);
		fprintf(stderr, "Error: Couldn't start \"%s\": %s\n",
							m->cmd, strerror(errno));
		exit(7);
	} else {
		pp->pid = new_pid;
		if ( app->last_terminated == new_pid ) remove_pid(new_pid);
		else {
			//printf("Starting [%s] (%d)\n", m->cmd, new_pid);
			//alarm(2);
		}
	}
}

void repaint( pMenuWindow win) {
	
	pMenuItem pi;
	pPid pp;
	int ypos, y = 0;
	
	if ( ! win) win = app->win;

	if ( ! app->win->extended ) {
		GrDrawImageBits(app->win->id, app->gc, 0, 0, &image_icon);
		return;
	}
	
	GrSetGCForeground( app->gc, WHITE);
	GrFillRect(win->id, app->gc, win->x, 0, win->width, win->height);
	
	y = -win->scroll_to;
	for ( pi = win->items; pi && (y<MAX_VISIBLE_ITEM); pi=pi->next) {
		
		if ( y >= 0) {

			GrSetGCForeground(app->gc, WHITE);
			GrFillRect(win->id, app->gc, 0, y*app->text_h, BORDER_X, app->text_h);

			if (((y==0) && (win->scroll_to>0)) || 
				((y == MAX_VISIBLE_ITEM-1) && ((y+win->scroll_to) < (win->nb_items-1)))) {

				GrSetGCForeground(app->gc, WHITE);
				GrFillRect(win->id, app->gc, 0, y*app->text_h, win->width, app->text_h);
				GrSetGCForeground(app->gc, BLACK);
				GrText( win->id, app->gc, BORDER_X, BORDER_Y + y*app->text_h, "...", 3, 0);
				
			} else {
				for( pp = app->pids; pp && (pp->item!=pi); pp = pp->next);
				if ( pp) {
					GrSetGCForeground(app->gc, GR_RGB(150,150,150));
					GrText( win->id, app->gc, BORDER_R, BORDER_Y + y*app->text_h, "*", 1, 0);
				}
				
				if ( (y == win->sel_item) && (pi->run_type!=RUN_TYPE_SEPARATOR) ) {
					if ( pp && (pi->run_type==RUN_TYPE_ONOFF))
						GrSetGCForeground( app->gc, RED);
					else if ( (pp && (pi->run_type==RUN_TYPE_ONCE)))
						GrSetGCForeground( app->gc, GR_RGB(250,250,250));
					else
						GrSetGCForeground( app->gc, GR_RGB(0,0,255));
				} else if (pi->sub_window)
						GrSetGCForeground( app->gc, GR_RGB(150,150,150));
					else
						GrSetGCForeground( app->gc, WHITE);
						
				GrFillRect(win->id, app->gc, BORDER_X, y*app->text_h, win->width - 2, app->text_h);

				if (pi->run_type==RUN_TYPE_SEPARATOR) {
					ypos = BORDER_Y + y*app->text_h - (app->text_h/2)+1;
					GrSetGCForeground( app->gc, BLACK);
					GrLine(win->id,app->gc,0,ypos, win->width, ypos);
				} else {
					if ( y == win->sel_item ) GrSetGCForeground( app->gc, WHITE);
					else if (pp && (pi->run_type==RUN_TYPE_ONOFF))	GrSetGCForeground( app->gc, BLACK);
					else if (pp && (pi->run_type==RUN_TYPE_ONCE)) GrSetGCForeground(app->gc, GR_RGB(150,150,150));
					else GrSetGCForeground( app->gc, BLACK);
					
					GrText( win->id, app->gc, BORDER_X, BORDER_Y + y*app->text_h, pi->name, strlen(pi->name), 0);
				}
			}
		}
		y++;
	}
}

inline int get_item_position( int y) {
	return ((y+1) / app->text_h);
}

void recalculate_win_size(pMenuWindow win) {

	pMenuItem pi;
	int w, h, b, max_w = -1;
	
	for( pi = win->items; pi; pi = pi->next) {
		if ( ! pi->name) continue;
		
		GrGetGCTextSize(app->gc, pi->name, strlen(pi->name), 0, &w, &h, &b);
		if ( max_w < w) max_w = w;
		if ( app->text_h < h ) app->text_h = h;
	}
	win->width = max_w + BORDER_X*2;
	win->height = ((win->nb_items>MAX_VISIBLE_ITEM)?MAX_VISIBLE_ITEM:win->nb_items) * app->text_h;
	if ( !win->height) win->height = app->text_h;
}

void set_extend( pMenuWindow win, int extended, int dx ) {
	GR_WM_PROPERTIES props;
		
	if ( ! win->id) {
		recalculate_win_size(win);
		win->id = GrNewWindowEx(GR_WM_PROPS_APPWINDOW|GR_WM_PROPS_NOAUTOMOVE, "nxMenu",GR_ROOT_WINDOW_ID,
						win->x = dx, 0, win->width, win->height, BLACK);
		props.flags = GR_WM_FLAGS_PROPS;
		props.props = GR_WM_PROPS_NODECORATE | GR_WM_PROPS_NOFOCUS;
		GrSetWMProperties(win->id, &props);
		GrSelectEvents(win->id, GR_EVENT_MASK_CLOSE_REQ |
			GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_MOUSE_MOTION |
			GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN);	
	}
	GrRaiseWindow( win->id);
	
	if ( extended ) {
		if ( win == app->win) GrResizeWindow( win->id, win->width, win->height);
		else GrMapWindow( win->id);
		
	} else {
		if ( win == app->win) GrResizeWindow( app->win->id, 16, 16);
		else GrUnmapWindow( win->id);
	}
	win->scroll_to = 0;
	win->next = 0;
	win->sel_item = -1;
	win->extended = extended;
}
	
	
int load_configuration( char *filename) {
	FILE *f;
	size_t len_buff;
	char *buff, *start, *type, *name, *cmd;
	int t, item_depth = 0;
	pMenuWindow depth[10];
	pMenuItem item;
	
	app->win = depth[0] = new_window();
	
	if ( filename) f = fopen(filename, "r");	
	if ( f) {
		buff = my_malloc(len_buff=1024);
		while ( getline(&buff, &len_buff, f) != -1) {

			for(start=buff; (*start) && (((*start)==' ')||(*start=='\t')); start++);
			if (*start == '#') continue;
			
			type = strtok(start, " \t");
			if ( type && name && (strncmp(type, "sub", 3)==0)) {
				name = strtok(NULL, "\n\r");

				if ( ! name) name = "(no name)  -->";
				else strcat(name, "  -->");
				item = add_item_to_window(depth[item_depth], new_submenu_item(name));
				item_depth++;
				depth[item_depth]= item->sub_window = new_window();
				continue;
			} else if (strncmp(type, "end", 3)==0) {
				item_depth--;
				continue;
			} else if (strncmp(type, "separator", 9)==0) {
				add_item_to_window(depth[item_depth], new_separator_item());
				continue;
			}
			
			name = strtok(NULL, "|");
			cmd = strtok(NULL, "\n\r");
			if ( type && name && cmd ) {
				
				while( ((*name) == ' ') || ((*name) == '\t')) name++;
				while( ((*cmd) == ' ') || ((*cmd) == '\t')) cmd++;
				if (strcmp(type, "once")==0) t = RUN_TYPE_ONCE;
				else if (strcmp(type, "onoff")==0) t = RUN_TYPE_ONOFF;
				else t = RUN_TYPE_NORMAL;
	
				add_item_to_window(depth[item_depth], new_program_item(name, t, cmd));
			} else if ( type)
				fprintf(stderr, "WARNING: ignoring wrong line : %s\t%s|%s\n", type, name, cmd);
		}
		fclose(f);
		free( buff);
		return 1;
	} else {
		fprintf(stderr, "Could not open configuration file %s\n", filename);
		return 0;
	}
}

void close_all_win_from( pMenuWindow win) {
	pMenuWindow win2;
	
	while ( win && win->extended) {
		win2 = win->next;
		set_extend(win, 0, win->x);
		win = win2;
	}
}

static void quit_prog( int sig) {
	killall();
	close_all_win_from( app->win);
	GrUnmapWindow(app->win->id);
	GrDestroyWindow(app->win->id);
	GrDestroyGC(app->gc);
	GrClose();
}

pMenuItem get_visible_item( pMenuWindow win, int n) {
	pMenuItem pi = win->items;
	int ni = get_item_position(n)+win->scroll_to;
	if ( (ni<0)||(ni>=win->nb_items)) return NULL;
	while (ni--) pi = pi->next;
	return pi;
}

pMenuWindow find_and_raise_window( int x, int y) {
	pMenuWindow nwin, win = app->win;
	pMenuItem i;
	
	while ( (win->next) && (x >= win->next->x)) win = win->next;
	if ((i = get_visible_item(win, y))) nwin = i->sub_window; else nwin = NULL;
	if (nwin != win->next) {
		close_all_win_from(win->next);
		if ( nwin) set_extend( win->next = nwin, 1, win->x + win->width - 1);
		else win->next = nwin;
	}
	return win;
}

pMenuWindow find_window_id( int id) {
	pMenuWindow win = app->win;
	while ( (win->id != id) && (win->next)) win=win->next;
	if ( win->id == id) return win; else return 0;
}

int main( int argc, char** argv) {
	
	//GR_WINDOW_INFO wininfo;
	GR_EVENT ev;
	int new_sel;
	int in_window;
	pMenuWindow win;
	pMenuItem item;

	if (GrOpen() < 0) {
		fprintf (stderr, "GrOpen failed");
		exit(1);
	}
		
	signal( SIGCHLD, &fils_action);
	signal( SIGQUIT, &quit_prog);
	signal( SIGINT, &quit_prog);
	signal( SIGTERM, &quit_prog);
	signal( SIGALRM, &alam_handler);
	
	app = my_malloc(sizeof(sAppData));
	app->null_fd = open("/dev/null", O_RDWR);
	
	app->gc = GrNewGC();
	GrSetGCUseBackground( app->gc, GR_FALSE);
	GrSetGCForeground( app->gc, BLACK);

/*	app->win->id = GrNewWindowEx(GR_WM_PROPS_APPWINDOW|GR_WM_PROPS_NOAUTOMOVE, "nxMenu",GR_ROOT_WINDOW_ID,
							0, 0, app->width = 50, app->height = 50, BLACK);
	props.flags = GR_WM_FLAGS_PROPS;
	props.props = GR_WM_PROPS_NODECORATE | GR_WM_PROPS_NOFOCUS;
	GrSetWMProperties(app->win->id, &props);
	GrSelectEvents(app->win->id, GR_EVENT_MASK_CLOSE_REQ |
			GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_MOUSE_MOTION |
			GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN);	
		*/
	//GrMoveWindow( app->pwid, 0, 0);
	load_configuration((argc==2)?argv[1]:"nanomenu.cfg");
	
	app->quit = in_window = 0;
	set_extend( app->win, 0, 0);
	GrMapWindow( app->win->id);

	//GrGetWindowInfo( app->win->id, &wininfo);
	//app->pwid = wininfo.parent;
	
	do {
		GrGetNextEvent(&ev);
		switch ( ev.type) {
			case GR_EVENT_TYPE_BUTTON_DOWN:
			case GR_EVENT_TYPE_MOUSE_MOTION:
					
					if ( !((GR_EVENT_MOUSE *)&ev)->buttons ) break;
					if ( ! app->win->extended) set_extend(win = app->win, 1, 0);
					
					win = find_and_raise_window(ev.mouse.rootx, ev.mouse.rooty);

					if ( ! win ) break;
					
					if ( (ev.mouse.rootx < win->x) || (ev.mouse.rooty < 0) || 
						 (ev.mouse.rootx > (win->x+win->width)) || (ev.mouse.rooty > win->height)) {
							 
						new_sel = -1;
					} else
						new_sel = get_item_position(ev.mouse.rooty);
						
					
					if ( (new_sel == 0) && (win->scroll_to>0) && step_elapsed()) {
						win->scroll_to--;
						win->sel_item = new_sel;
						repaint(win);
					}
					
					else if ( (new_sel == (MAX_VISIBLE_ITEM -1)) && 
						 ((new_sel+win->scroll_to)<(win->nb_items -1)) && step_elapsed()) {
							 
						win->scroll_to++;
						win->sel_item = new_sel;
						repaint(win);
					}
					else if ( win->sel_item != new_sel ) {
 
						win->sel_item = new_sel;
						repaint(win);
					}
					break;
					
			/* case GR_EVENT_TYPE_UPDATE: */
			case GR_EVENT_TYPE_EXPOSURE:
					repaint(find_window_id(((GR_EVENT_EXPOSURE*)&ev)->wid));
					break;
					
			case GR_EVENT_TYPE_BUTTON_UP: 
					win = find_and_raise_window(ev.mouse.rootx, ev.mouse.rooty);
					
					item = get_visible_item(win, ev.mouse.rooty);
					if ( item) run_item( item);
					
					close_all_win_from(app->win);
					set_extend(app->win, 0, 0);
					break;
					
			/*case GR_EVENT_TYPE_BUTTON_DOWN:
					set_extend(win = app->win, 1, 0);
					win = find_and_raise_window(ev.mouse.rootx, ev.mouse.rooty);
					if ( ! win) break;
					
					win->sel_item = get_item_position(ev.mouse.rooty);
					repaint(win);
					break;
			*/		
			case GR_EVENT_TYPE_CLOSE_REQ: 
					killall();
					app->quit = 1;				
		}
	} while ( ! app->quit);
	
	quit_prog(0);
	return 0;
}
