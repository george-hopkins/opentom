#! /bin/sh

level=`flmessage -y -i -t "NanoBrick - New Level ?" "Choose a name :"`

if [ ! -z "$level" ]
then
	cd $DIST/share/nanobrick_levels
	echo "gogo=$level=" >/tmp/toto
	nanobrick_editor -l "$level"
fi
