#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h> 
#include <linux/types.h>
#include <stdio.h>
#include <barcelona/Barc_Gpio.h>

int pollStatus(int devHWStatus)
{
	HARDWARE_STATUS hwstatus;

	if(ioctl(devHWStatus, IOR_HWSTATUS, &hwstatus) < 0) {
		fprintf(stderr, "could not read hardware status: %s", strerror(errno));
		return -1;
	}

	return hwstatus.u8InputStatus;
}

int main(int argc, char **argv) {
	UINT32 status = 0;
	
	int fd = open("/dev/hwstatus", O_RDONLY);
	if (fd == -1)
    {
        perror("/dev/hwstatus");
        return 2;
    }
    
    if ( argc > 1) {
		if ( strncmp(argv[1], "-r", 2) == 0) {
			if(ioctl(fd, IOW_RESET_ONOFF_STATE, &status) < 0) {
				fprintf(stderr, "could not read hardware status: %s\n", strerror(errno));
				return -1;
			} else printf("Reseting ONOFF STATE: OK\n");
				
		} else printf("Usage: %s [-r]\n", argv[0]);
	} else {
		printf("Input status : %d\n", pollStatus( fd));
	}
    return 0;
    
}
