#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
 
int main(int argc, char* argv[])
{
    int fd;

    if((fd = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr, "could not open watchdog: %s", strerror(errno));
        return 1;
    }

    while(1) {
        write(fd, "\0", 1);
        sleep(15);               // Perhaps 13 would be safer...
    }
}
