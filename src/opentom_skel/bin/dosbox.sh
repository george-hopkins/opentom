#! /bin/sh

cd $DIST/share/dosbox
exec nice -n 19 $DIST/bin/dosbox -conf $1_dosbox.conf
