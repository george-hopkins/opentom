#! /bin/sh

export DIST=/mnt/sdcard/opentom
export HOME=$DIST

export FRAMEBUFFER=/dev/fb

export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=$DIST/etc/ts.conf
export TSLIB_PLUGINDIR=$DIST/lib/ts
export TSLIB_CALIBFILE=$DIST/etc/pointercal

export PATH=$PATH:$DIST/bin
export LD_LIBRARY_PATH=$DIST/lib

ln -s $DIST/lib/libz.so.1 /lib/libz.so

if [ ! -e /etc/profile ]; then ln -s $DIST/etc/profile /etc/profile; fi

echo "Disabling BT"
stop_bt -s

ifconfig lo 127.0.0.1 up

cd /dev
ln -s fb fb0
export NANOX_YRES=`fbset -s | grep geometry | if read x x yres x; then echo $yres; fi`

# Verify TS is calibrated
if [ ! -f $TSLIB_CALIBFILE ]; then ts_calibrate; fi

cd $DIST

# Suspend when the power button is pressed or the battery is low
power_button -b bin/suspend bin/suspend &

while /bin/true
do
	sleep 1
	pidof nano-X || { 
		nice -n -10 nano-X &
		nanowm &
	}
	sleep 1
	nxmenu $DIST/etc/nxmenu.cfg >$DIST/logs/nxmenu.log 2>&1
done

