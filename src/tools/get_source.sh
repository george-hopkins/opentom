#! /bin/sh

exist=`ls ${DOWNLOADS}/*$1* 2>/dev/null`
if [ ! -z "$exist" ]; then
#	ln -s "$exist" ${DOWNLOADS}/$1 2>/dev/null
	exit 0;
fi

echo Searching how to get $1 ...

source=`grep "$1" $CONFIGS/sources.txt | grep -v ^\#`

if [ -z "$source" ]; then
	echo URL to get "$1" not found in $CONFIGS/sources.txt
	exit 1;
else
	cd ${DOWNLOADS}
	echo Running : \"$source\" ...
	eval "$source"
fi
