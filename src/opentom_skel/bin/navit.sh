#! /bin/sh

export LANG=en_US.utf8

# Navit can use SiRFstar III GPS (device id 3; used in TT7xx) without gltt
# For GPS chips without NMEA output gltt is used
if [ `cat /proc/barcelona/gpstype` == 3 ] ; then
	ln -sf /dev/`cat /proc/barcelona/gpsdev` /var/run/gpspipe
else
	ls $DIST/bin/gltt || {
		echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit"
		exit
	}

	if [ -z `pidof gltt` ]; then rc.gltt start; fi
	echo EnableRawGPSOutput >/dev/gps
fi

if [ "`cat /proc/barcelona/tfttype`" == 2 ] || [ "`cat /proc/barcelona/tfttype`" == 3 ] ; then
	NAVIT_CONFIG=$DIST/share/navit/navit.xml
else
	NAVIT_CONFIG=$DIST/share/navit/navit_xl.xml
fi

navit -c $NAVIT_CONFIG $* >$DIST/logs/navit.log 2>&1

if [ ! -z `pidof gltt` ]; then rc.gltt stop; fi
