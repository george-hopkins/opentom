#! /bin/sh

if [ x`cat /proc/barcelona/gpstype` == x3 ]
then 
    ln -sf `cat /proc/barcelona/gpsdev` /dev/gpsdata
else
ls $DIST/bin/gltt || {
	echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit"
	exit
}
if [ -z `pidof gltt` ]; then rc.gltt start; fi
echo EnableRawGPSOutput >/dev/gps
fi

export LANG=de_DE.utf8

export NAVIT_PREFIX=$DIST
export NAVIT_LIBDIR=$DIST/lib/navit
export NAVIT_SHAREDIR=$DIST/share/navit

navit -c $NAVIT_SHAREDIR/navit$NANOX_YRES.xml $* >$DIST/logs/navit.log 2>&1
rc.gltt stop


