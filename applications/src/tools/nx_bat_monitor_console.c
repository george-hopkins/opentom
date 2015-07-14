#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
 
#include <barcelona/Barc_Battery.h>

// voir la doc applications/doc/ I/Oo control in Linux.html
 
void get_vars(int fd)
{
    BATTERY_STATUS s;
 
    if (ioctl(fd, IOR_BATTERY_STATUS, &s) == -1)
    {
        perror("IOR_BATTERY_STATUS");
    }
    else
    {
        printf("Battery Voltage = %d\nCharge Current = %d\nChargeStatus = %d\n", s.u16BatteryVoltage, s.u16ChargeCurrent, s.u8ChargeStatus);
    }
}

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
    char *file_name = "/dev/battery";
    int fd;
    enum
    {
        e_get,
        e_clr,
        e_set
    } option;
 
    if (argc == 1)
    {
        option = e_get;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-g") == 0)
        {
            option = e_get;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            option = e_clr;
        }
        else if (strcmp(argv[1], "-s") == 0)
        {
            option = e_set;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
        return 1;
    }
    fd = open(file_name, O_RDWR);
    if (fd == -1)
    {
        perror("query_apps open");
        return 2;
    }
 
    switch (option)
    {
        case e_get:
            get_vars(fd);
            break;
        case e_clr:
            disable_charging(fd);
            break;
        case e_set:
            enable_charge(fd);
            break;
        default:
            break;
    }
 
    close (fd);
 
    return 0;
}
