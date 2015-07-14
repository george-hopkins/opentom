#! /bin/sh

export LANG=fr_FR.utf8

export NAVIT_PREFIX=$DIST
export NAVIT_LIBDIR=$DIST/lib/navit
export NAVIT_SHAREDIR=$DIST/share/navit

if [ -z `pidof gltt` ]; then rc.gltt start; fi
echo EnableRawGPSOutput >/dev/gps

navit -c $NAVIT_SHAREDIR/navit$NANOX_YRES.xml $* >$DIST/logs/navit.log 2>&1
rc.gltt stop


