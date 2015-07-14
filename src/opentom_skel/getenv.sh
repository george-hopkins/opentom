export DIST=/mnt/sdcard/opentom
export HOME=$DIST

export NANOX_YRES=272

export FRAMEBUFFER=/dev/fb
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=$DIST/etc/ts.conf
export TSLIB_PLUGINDIR=$DIST/lib/ts
export TSLIB_CALIBFILE=$DIST/etc/pointercal

export PATH=/sbin:/usr/sbin:/bin:/usr/bin:$DIST/bin
export LD_LIBRARY_PATH=$DIST/lib

