#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MWINCLUDECOLORS
#include <microwin/nano-X.h>

#define HEIGHT 40

char *bri_name = "/sys/class/backlight/s3c/brightness";

int read_val(char *filename) {
	char buff[10];
	int len;
	
	int fd = open(filename, O_RDONLY);
	if ( (len=read(fd, &buff, 9)) <= 0) {
		perror(filename);
		return -1;
	} else {
		buff[len]=0;
		printf("read_val = '%s'\n", buff);
		return atoi(buff);
	}
}


int main()
{
	int fd = open(bri_name, O_WRONLY);
	if ( fd < 0) {
		perror(bri_name);
		return 1;
	}
	
	char buff[10];
	int notquit = 1, val= read_val(bri_name), max = read_val("/sys/class/backlight/s3c/max_brightness");
	
	if ( max <= 0) return 1;
	
	GR_WINDOW_ID	wid;		/* window id */
	GR_GC_ID	gc;		/* graphics context id */
	GR_EVENT	event;		/* current event */
	GR_SCREEN_INFO	si;		/* screen information */

	if (GrOpen() < 0) {
		fprintf(stderr, "Cannot open graphics\n");
		return 1;
	}

	GrGetScreenInfo(&si);
	printf("R=%d C=%d\n", si.rows, si.cols);

	wid = GrNewWindow(GR_ROOT_WINDOW_ID, 50, 50, max, HEIGHT, 1, BLACK, WHITE);
		
	GR_WM_PROPERTIES props;
	props.title = "Nx Light";
	props.flags = GR_WM_FLAGS_TITLE;
	GrSetWMProperties( wid, &props);

	GrSelectEvents(wid, GR_EVENT_MASK_CLOSE_REQ |
			GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_MOUSE_MOTION |
			GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN );
	GrMapWindow(wid);
	gc = GrNewGC();

	while (notquit) {
		GrGetNextEvent(&event);
		switch (event.type) {
			case GR_EVENT_TYPE_MOUSE_MOTION:
					if ( !((GR_EVENT_MOUSE *)&event)->buttons ) break;
					if ( (event.mouse.x > 0) && (event.mouse.x < max)) {
						snprintf(buff, 9, "%d\n", val = event.mouse.x);
						write( fd, buff, strlen(buff));
					}
					GrSetGCForeground(gc, WHITE);
					GrFillRect(wid, gc, 0, 0, val, HEIGHT);
					GrSetGCForeground(gc, BLACK);
					GrFillRect(wid, gc, val, 0, max+1, HEIGHT);
					
					break;
			case GR_EVENT_TYPE_EXPOSURE:
				if (event.exposure.wid == wid)
					GrSetGCForeground(gc, WHITE);
					GrFillRect(wid, gc, 0, 0, val, HEIGHT);
					GrSetGCForeground(gc, BLACK);
					GrFillRect(wid, gc, val, 0, max+1, HEIGHT);
				break;
			case GR_EVENT_TYPE_CLOSE_REQ: 
				notquit = 0;
		}
	}
	close(fd);
	return 0;
}
