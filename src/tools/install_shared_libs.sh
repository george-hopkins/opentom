#! /bin/bash

if [ -z "$1" ]; then
	echo Usage: $0 <arm-sysroot> \"lib_path1 lib_path2 ...\"
	exit
fi

initramfs_dir=$1
shift
paths=$*

typeset -i found

find $initramfs_dir -type f -exec ${T_ARCH}-readelf -d {} 2>/dev/null \; | grep NEEDED | cut -f2 -d[ | cut -f1 -d] | sort | uniq | while read lib
do

	if [ -f $initramfs_dir/lib/$lib ]; then
		echo $lib is allready installed.
		continue
	fi

	found=0
	for p in $paths
	do
		if [ -f $p/$lib ];
		then
			found=1
			echo found $p/$lib
			cp -u $p/$lib $initramfs_dir/lib/
			${CROSS}/bin/${STRIP} $initramfs_dir/lib/$lib
			break
		fi
	done

	if [[ $found -eq 0 ]]; then
		echo WARNING: Can\'t find : $lib for this files :
		find $initramfs_dir -type f -exec is_use_lib.sh {} $lib \; -print
	fi

done

