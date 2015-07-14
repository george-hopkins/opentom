#! /bin/sh

RC_FILE=$DIST/share/gnuboy/classic.rc
DISPLAY=:0 exec gnuboy --source $RC_FILE $*
