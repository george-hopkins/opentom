#!/bin/sh
# GPS Dashboard launcher: starts the GPS receiver, runs the dashboard, then
# stops the receiver again so it does not drain the battery.
#
# Only GPS chip type 130 ("Barracuda", e.g. TomTom XL LIVE) is supported. It
# needs TomTom's own glgps and its config file, taken from your own device:
#     /mnt/sdcard/bin/glgps                  (already on the device storage)
#     /mnt/sdcard/gl/cfg/glconfig.xml        (etc/glconfig.xml from the ramdisk
#                                             of the original ttsystem)
# They are not part of OpenTom and are not redistributed. See the README.

DIST=${DIST:-/mnt/sdcard/opentom}
SDCARD=/mnt/sdcard

fail() {
	echo "$1" | flmessage -s -t "Can't start GPS Dashboard"
	exit 1
}

GPSTYPE=`cat /proc/barcelona/gpstype`
[ "$GPSTYPE" = 130 ] || fail "GPS chip type $GPSTYPE is not supported yet (only type 130 is)."

[ -f $SDCARD/bin/glgps ] || fail "glgps not found in $SDCARD/bin. Please read the README."
[ -f $SDCARD/gl/cfg/glconfig.xml ] || [ -f /etc/glconfig.xml ] || fail "glconfig.xml not found. Please read the README."

ln -sf `cat /proc/barcelona/gpsdev` /dev/gpsdata
mkdir -p /var/run/gl
[ -p /var/run/gpspipe ] || mknod /var/run/gpspipe p

if [ -z "`pidof glgps`" ]; then
	$SDCARD/bin/glgps normal >/tmp/glgps.log 2>&1 &
	sleep 2
fi

$DIST/bin/gpsdash

killall glgps 2>/dev/null
sleep 2
killall -9 glgps 2>/dev/null
exit 0
