#! /bin/sh

export LANG=en_US.utf8
export NAVIT_SHAREDIR="$DIST/share/navit"
export TZ="CEDT-01:00:00CEST-02:00:00,M3.4.0,M10.4.0"
export LANG=en_US
export ESPEAK_DATA_PATH="$DIST/mnt/sdcard/navit/share/navit"

# Navit can use SiRFstar III GPS (device id 3; used in TT7xx) without gltt
# For GPS chips without NMEA output gltt is used
[ "$hw_gpstype" == 3 ] && ln -sf "/dev/$hw_gpsdev" /var/run/gpspipe || {
	[ ! -x "$DIST/bin/gltt" ] && echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit" && exit
} || ! rc.gltt start || echo EnableRawGPSOutput >/dev/gps

NAVIT_CONFIG="$DIST/share/navit/navit.xml"

navit -c "$NAVIT_CONFIG" "$@" >"$DIST/logs/navit.log" 2>&1

[ "$(pidof gltt)" ] && rc.gltt stop
