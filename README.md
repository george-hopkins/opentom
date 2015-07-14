# OpenTom

**OpenTom** is a tiny, open source Linux distribution for TomTom™ devices.


## How to use this SDK

- Install the following dependencies: subversion chrpath fluid imagemagick
- Set the `ROOT` envvar in `get_cross_env.sh`
- `source get_cross_env.sh`
- Run `make` to start the initial OpenTom build
- This may take a while; consider getting yourself a coffee ;-)
- Copy `build/ttsystem` (boot image) to the root folder of your SD card (backup the original one first!)
- Copy  the contents of `opentom_dist/` to a (new) folder called `opentom` on your SD card


## How to build extra applications
- Run `make extra` and copy the files as described above
- For dosbox, dune2, gnuboy, linapple, and scummvm games: Take it from internet and copying it into opentom/share subdirectories.
- For Navit, you need the TomTom gltt (see below, that read raw GPS data from /dev/gps and send it to /var/run/gpspipe).


## How to modify `ttsystem`

- For kernel: `cd kernel; make menuconfig`
- For busybox: `cd build/busybox*; make menuconfig`
- For initramfs: do your changes and `touch initramfs/etc/rc`
- Then: return to `$ROOT` and run `make ttsystem`


## How to add some new applications

- Just extract you source into $ROOT/src (for libraries) or applications/src (or build if no patch to apply) or into build directory for existing source
- type: configure --prefix=$ARM_APPROOT --host=$T_ARCH (if possible)
- copy the final executable into $(TOMDIST)/bin (update Makefile to automate this ?)
- type: make verif_dist (in $ROOT dir) to update used shared libs on opentom_dist
- copy it on your TomTom
- if it works as you want, make a patch (with make patch-<my_app_dir_name>), update applications/Makefile

### On the TomTom side
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


## How to install gltt from TomTom `ttsystem` file (for Navit)
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
- Then: cp $ROOT/src/ttsystem.tomtom.initramfs/bin/gltt $TOMDIST/bin


## To-Do

- fix espeak => portaudio => OSS, that currently don't work
- patch spreadsheet to be adapted to TomTom screen


## Support

- Checkout our documentation in `docs/`
- Take a look at the shell scripts, Makefiles, and patches
- Feel free to send an email in case of a problem or if you found betters URLs for sources.txt (opentom@free.fr).


## Authors

- Clément (opentom@free.fr)
