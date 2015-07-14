#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
 
#define MWINCLUDECOLORS
#include <microwin/nano-X.h>

#include <barcelona/Barc_Battery.h>
char *bat_status[] = { "NO_POWER", "COMPLET", "CHARGING" };
 
void enable_charge(int fd)
{
    if (ioctl(fd, IO_ENABLE_CHARGING) == -1)
    {
        perror("IO_ENABLE_CHARGING");
    }
}

void disable_charging(int fd)
{
	if (ioctl(fd, IO_DISABLE_CHARGING) == -1)
    {
        perror("IO_DISABLE_CHARGING");
    }
}
 
int main(int argc, char *argv[])
{   
	if (GrOpen() < 0) {
		fprintf(stderr, "Cannot open graphics\n");
		return 1;
	}
	
    int fd = open("/dev/battery", O_RDWR);
    if (fd == -1)
    {
        perror("/dev/battery");
        return 1;
    }
    BATTERY_STATUS s;
 	ioctl(fd, IOR_BATTERY_STATUS, &s);
 	
	GR_WINDOW_ID	wid;		/* window id */
	GR_GC_ID	gc;		/* graphics context id */
	GR_EVENT	event;		/* current event */
	GR_WINDOW_INFO info;
	
	char buff[1024];
	int i, notquit = 1;

	wid = GrNewWindow(GR_ROOT_WINDOW_ID, 50, 50, 165, 55, 1, BLACK, WHITE);
	GR_WM_PROPERTIES props;
	props.title = "Nx Battery";
	props.flags = GR_WM_FLAGS_TITLE;
	GrSetWMProperties( wid, &props);
	GrSelectEvents(wid, GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_EXPOSURE);
	GrMapWindow(wid);
	gc = GrNewGC();

	while (notquit) {
		GrGetNextEventTimeout(&event, 5000);
		switch (event.type) {
			case GR_EVENT_TYPE_CLOSE_REQ: 
				notquit = 0;	
				break;				
			case GR_EVENT_TYPE_TIMEOUT:
				ioctl(fd, IOR_BATTERY_STATUS, &s);
			case GR_EVENT_TYPE_EXPOSURE:
					GrGetWindowInfo(wid, &info);
					GrSetGCBackground(gc, WHITE);
					GrSetGCForeground(gc, WHITE);
					GrFillRect(wid, gc, 0, 0, info.width, info.height);
					GrSetGCForeground(gc, BLACK);
					
					if (ioctl(fd, IOR_BATTERY_STATUS, &s) == -1) {
						sprintf(buff,"IOR_BATTERY_STATUS");
						
					} else
						for ( i = 0; i < 3; i++) {
							switch (i) {
								case 0: sprintf(buff, "Battery Voltage = %d mV", s.u16BatteryVoltage);
										break;
								case 1: sprintf(buff, "Charge Current = %d mA", s.u16ChargeCurrent);
										break;
								case 2: sprintf(buff, "ChargeStatus = (%d) %s", s.u8ChargeStatus, bat_status[s.u8ChargeStatus]);
							}
							GrText(wid, gc, 5, 15+i*15, buff, strlen(buff), MWTF_ASCII);
					}
		}
	}
	close(fd);
	return 0;
}
