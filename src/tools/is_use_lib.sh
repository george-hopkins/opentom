#! /bin/sh

if ${T_ARCH}-readelf -d $1 2>/dev/null | grep NEEDED | cut -f2 -d[ | cut -f1 -d] | grep $2 >/dev/null
then
	exit 0
else
	exit 1
fi
