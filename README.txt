OpenTom SDK Readme.txt

OpenTom is a tiny OpenSource Linux distribution for TomTom(tm)

* To use this SDK :
- you need package (in your system): subversion chrpath fluid imagemagick
- modify the ROOT env var into get_cross_env.sh
- source get_cross_env.sh
- type make to perform primary OpenTom buids

* To build (into opentom_dist) extra application
- mount your SDCARD on /mnt/sdcard or link $ROOT/opentom_dist to /mnt/sdcard/opentom (for espeak-data)
- type: make extra

Then, you will have :
- ttsystem boot image into the build directory (before copying it on TomTom SD cars, BACKUP the original ttsystem first !),
- and a opentom_dist directory (to copy on your tomtom as opentom).

** NB:
- For dosbox, dune2, gnuboy, linapple, and scummvm games: take it from internet and copying it into opentom/share subdirectories.
- For Navit, you need the TomTom gltt (see below, that read raw GPS data from /dev/gps and send it to /var/run/gpspipe).


* To modify ttsystem :
- for kernel: cd kernel; make menuconfig
- for busybox: cd build/busybox*; make menuconfig
- for initramfs: do your changes and touch initramfs/etc/rc
- Then: return to $ROOT ans type : make ttsystem


* To add some new applications :

- Just extract you source into $ROOT/src (for libraries) or applications/src (or build if no patch to apply)
	or into build directory for existing source
- type: configure --prefix=$ARM_APPROOT --host=$T_ARCH (if possible)
- copy the final executable into $(TOMDIST)/bin (update Makefile to automate this ?)
- type: make verif_dist (in $ROOT dir) to update used shared libs on opentom_dist
- copy it on your TomTom
- if it works as you want, make a patch (with make patch-<my_app_dir_name>), update applications/Makefile

* On the TomTom side :
- When you boot your TomTom with OpenTom ttsystem, you can directly use Telnet to loggin in as root from USB
- Use FTP server to update your files
- strace and gdb are ready to be used to debug you programs

* Creating Nano-X test platform on your system:
- create $ROOT/i386 directory, extract, configure, install : microwin, nxlib (with libNX11), SDL, Fltk ...
- configure the all with LDFLAGS=-L/usr/local/lib --prefix=/usr/local
- try NetBeans to perform you developments ?

* To free some memory (~3-4Mo on 32 !) : use an ext2 partion on your SDcard to remplace (and free) initramfs with buzybox pivot_root/chroot :
- use fdisk to create two partitions on your SDCARD : partion1=vfat(TomTom), partition2=ext2(10Mo?)
- copy ttsystem into SD.part1 and verify it boot, if not try to copy gns and program directory from original TomTom and others...
- when it boot :
- verify that busybox include chroot and pivot_root,
- copy the unmodified initramfs/* into linux SD partition, with /var/* linked on /tmp
- copy the configs/etc_rc_ext2 to SD.part2/etc/rc
- verify that kernel include ext2 filesystem support,
- copy configs/etc_rc_file.pivot_root_ext2 to initramfs/etc/rc
- make ttsystem
- then on partition 1 copy the built/ttsystem and the opentom dist directory on SD.part1
- if something goes wrong, try configs/kernel_config.console_ext2 to activate kernel FrameBuffer console

* To install gltt from TomTom ttsystem file (for Navit)
- copy your TomTom(tm) ttsystem file into $ROOT/src
	eg: cp /mnt/TOMTOM/ttsystem $ROOT/src/ttsystem.tomtom
- type :
	cd src
	ttimgextract ttsystem.tomtom 
	mkdir -p ttsystem.tomtom.initramfs
	cd ttsystem.tomtom.initramfs
	gunzip -c ../ttsystem.tomtom.0 | sudo cpio -i
- now the boot ramdisk of your tomtom is extracted in $ROOT/src/ttsystem.tomtom.initramfs
	and the TomTom kernel is $ROOT/src/ttsystem.1

Then: cp $ROOT/src/ttsystem.tomtom.initramfs/bin/gltt $TOMDIST/bin


* TODO
- fix espeak => portaudio => OSS, that currently don't work
- patch spreadsheet to be adapted to TomTom screen


* In case of problem
- read specific cases in docs directory,
- take a tour into shell scripts, Makefiles, and patchs for more informations,
- feel free to send me a mail in case of problem or found betters URLs for sources.txt (opentom@free.fr).

Have fun,

Clément.

