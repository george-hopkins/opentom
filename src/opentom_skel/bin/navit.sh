#! /bin/sh

ls $DIST/bin/gltt || {
	echo "You need TomTom(tm) gltt\nPlease read documentation." | flmessage -s -t "Can't start Navit"
	exit
}

export LANG=en_US.utf8

if [ -z `pidof gltt` ]; then rc.gltt start; fi
echo EnableRawGPSOutput >/dev/gps

navit -c $DIST/share/navit/navit.xml $* >$DIST/logs/navit.log 2>&1

rc.gltt stop
