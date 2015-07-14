#! /bin/sh

killall -STOP nxmenu
killall -QUIT nano-X
ts_calibrate
killall -CONT nxmenu
