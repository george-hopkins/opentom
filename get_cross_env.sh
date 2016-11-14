# Source this file to use OpenTomSDK

export ROOT=
export SOURCE="$ROOT/src"
export INITRAMFS_ROOT="$ROOT/initramfs"
export ARM_ROOT=arm-sysroot
export ARM_SYSROOT="$ROOT/$ARM_ROOT"
export ARM_APPROOT="$ARM_SYSROOT/usr"
export TOMDIST="$ROOT/opentom_dist"
export DOWNLOADS="$ROOT/Downloads"
export CONFIGS="$ROOT/configs"
export PKG_CONFIG_LIBDIR=$ARM_APPROOT/lib
export PKG_CONFIG_PATH=$ARM_APPROOT/lib/pkgconfig
export DOWNLOADS=Downloads

# Choix du Cross GCC
#if [ "old" == "$1" ]; then

	export TOOLCHAIN=old
	echo Utilisation de gcc-3.3.4_glibc-2.3.2
	export CROSS="$ROOT/gcc-3.3.4_glibc-2.3.2"
	# obligatoire pour ce GCC (je pense qu'il utilise des sys-root/../../../lib_du_compilo)
	export PREFIX="$CROSS/arm-linux/sys-root"
	export T_ARCH=arm-linux
	unset MARCH
#	export CFLAGS="-mlittle-endian -march=armv4 -mtune=arm9tdmi -mshort-load-bytes -fno-omit-frame-pointer -fno-optimize-sibling-calls -mno-thumb-interwork -O2 -I$PREFIX/include -I$PREFIX/usr/include"
#	export CPPFLAGS="-I$PREFIX/include -I$PREFIX/usr/include"
#	export LDFLAGS="-L$PREFIX/lib -L$PREFIX/usr/lib"
	export CFLAGS="-march=armv5te -mtune=arm9tdmi"
	export CPPFLAGS="-march=armv5te -mtune=arm9tdmi"
	unset LDFLAGS
	unset CONFIG_SYSROOT
#else
#	export TOOLCHAIN=new
#	echo Utilisation de Sourcery_G++_Lite
#	export PREFIX="$ROOT/arm-root"
#	export CROSS="$ROOT/Sourcery_G++_Lite"
#	#export CROSS="$ROOT/arm-2009q1"
#	export T_ARCH=arm-none-linux-gnueabi
#	MARCH="-march=armv4 -mtune=arm9tdmi -mlittle-endian"
#	export CFLAGS="$MARCH --sysroot=$PREFIX -Wl,--sysroot=$PREFIX -O2 -I$PREFIX/include -I$PREFIX/usr/include"
#	export CPPFLAGS="$MARCH -I$PREFIX/include -I$PREFIX/usr/include"
#	export LDFLAGS="--sysroot=$PREFIX -L$PREFIX/lib -L$PREFIX/usr/lib"
#	export CONFIG_SYSROOT=$PREFIX
#fi

export PATH=$ARM_APPROOT/bin:/usr/bin:/bin:$ROOT/tools:$CROSS/bin

export CC="${T_ARCH}-gcc"
export CXX="${T_ARCH}-g++"
export LD="${T_ARCH}-ld"
export NM="${T_ARCH}-nm -B"
export AR="${T_ARCH}-ar"
export RANLIB="${T_ARCH}-ranlib"
export STRIP="${T_ARCH}-strip"
export OBJCOPY="${T_ARCH}-objcopy"
export LN_S="/bin/ln -s"
export AS="${T_ARCH}-as"

export CROSS_COMPILE="${T_ARCH}-"
export ARCH=arm
export INSTALL_MOD_PATH=${INITRAMFS_ROOT}/usr

# Pour les progs utilisant libtool et son problème de sysroot, créer un script dans /usr/local/bin/${T_ARCH}-gcc
# qui contient
# #! /bin/bash
# exec ${CROSS}/bin/${T_ARCH}-gcc --arm-root=$PREFIX $*
#
# Changer PATH=/usr/local/bin:${CROSS}/bin:/usr/bin:/bin
