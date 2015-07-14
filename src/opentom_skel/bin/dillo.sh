#! /bin/sh

HOME=$DIST
ifconfig lo 127.0.0.1 up

dpid &
sleep 2
dillo $*
killall dpid
