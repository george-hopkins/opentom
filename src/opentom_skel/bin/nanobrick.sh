#! /bin/sh


if [ -z "$1" ]
then
	cd $DIST/share/nanobrick_levels
	flxplorer
else
	cd ${1%/*}
	level=${1##*/}
	nanobrick -l ${level%.blocks}
fi 
