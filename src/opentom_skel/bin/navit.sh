#! /bin/sh


export TZ="GMT-1"
export LANG=de_DE

export NAVIT_PREFIX=$DIST
export NAVIT_LIBDIR=$DIST/lib/navit
export NAVIT_SHAREDIR=$DIST/share/navit
export NAVIT_LOCALEDIR=$DIST/navit/share/locale

#Navit can use SiRFstar III GPS (DEVICE ID 3 ) used in TT7xx without gltt
if [ `cat /proc/barcelona/gpstype` == 3 ] ; then

	ln -sf /dev/`cat /proc/barcelona/gpsdev` /var/run/gpspipe 
else

#For other GPS without nmea output gltt is used

	rm /ver/run/gpspipe
	ls $DIST/bin/gltt || {
		echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit"
		exit
	}

	if [ -z `pidof gltt` ]; then rc.gltt start; fi
	echo EnableRawGPSOutput >/dev/gps
fi

set  >$DIST/logs/navite.log
navit -c $NAVIT_SHAREDIR/navit$NANOX_YRES.xml $* >$DIST/logs/navit.log 2>&1
 if [ ! -z `pidof gltt` ]; then rc.gltt stop; fi


