#! /bin/sh

ls $DIST/bin/gltt || {
	echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit"
	exit
}

export LANG=en_US.utf8

if [ -z `pidof gltt` ]; then rc.gltt start; fi
echo EnableRawGPSOutput >/dev/gps

if [ "`cat /proc/barcelona/tfttype`" == 2 ] || [ "`cat /proc/barcelona/tfttype`" == 3 ] ; then
	NAVIT_CONFIG=$DIST/share/navit/navit.xml
else
	NAVIT_CONFIG=$DIST/share/navit/navit_xl.xml
fi

navit -c $NAVIT_CONFIG $* >$DIST/logs/navit.log 2>&1

rc.gltt stop
