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

ln -s $DIST/bin/dbclient /usr/bin/dbclient
ln -s $DIST/bin/scp /usr/bin/scp
ln -s $DIST/lib/libz.so.1 /lib/libz.so

echo "Disabling BT"
stop_bt -s

ifconfig lo 127.0.0.1 up

cd /dev
ln -s fb fb0
export NANOX_YRES=`fbset -s | grep geometry | if read x x yres x; then echo $yres; fi`

# Verify TS is calibrated
if [ ! -f $TSLIB_CALIBFILE ]; then ts_calibrate; fi

# make ". s" usable to get environment in telnet 
cp $DIST/getenv.sh /s
echo cd $DIST >> /s

cd $DIST
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

