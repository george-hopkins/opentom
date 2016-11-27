# OpenTom

[![Build Status](https://img.shields.io/circleci/project/github/george-hopkins/opentom/master.svg)](https://circleci.com/gh/george-hopkins/opentom)
[![Latest Pre-Built Image](https://img.shields.io/badge/pre--built%20image-latest-green.svg)](https://circleci.com/api/v1/project/george-hopkins/opentom/latest/artifacts/0//home/ubuntu/opentom/target/opentom.tar.gz?branch=master&filter=successful)

**OpenTom** is a tiny, open source Linux distribution for TomTom™ devices.


## Getting Started

- Install the following dependencies: subversion chrpath fluid imagemagick xsltproc
- Set the `ROOT` envvar in `get_cross_env.sh`
- `source get_cross_env.sh`
- Run `make` to start the initial OpenTom build
- This may take a while; consider getting yourself a coffee ;-)
- Copy `build/ttsystem` (boot image) to the root folder of your SD card (backup the original one first!)
- Copy the contents of `opentom_dist/` to a (new) folder called `opentom` on your SD card


## How to build extra applications
- Run `make extra` and copy the files as described above
- For dosbox, dune2, gnuboy, linapple, and scummvm games: Take it from internet and copying it into opentom/share subdirectories.
- For coolreader: Run `sudo updatedb` in case the default font is not found.
- For Navit, you need the TomTom gltt (see below, that read raw GPS data from /dev/gps and send it to /var/run/gpspipe).


## How to modify `ttsystem`

- For kernel: `cd kernel; make menuconfig`
- For busybox: `cd build/busybox*; make menuconfig`
- For initramfs: do your changes and `touch initramfs/etc/rc`
- Then: return to `$ROOT` and run `make ttsystem`


## How to add some new applications

- Just extract you source into `$ROOT/src` (for libraries) or `applications/src` (or `build` if no patches should be applied)
- Run `./configure --prefix=$ARM_APPROOT --host=$T_ARCH` (adapt accordingly in case the project is not based on Autoconf)
- Copy the final executable into `$(TOMDIST)/bin`
- Rune `make verif_dist` (inside `$ROOT` directory) to update used shared libs in `opentom_dist`
- Copy the files to your TomTom device
- If it works as you wish, make a patch (with `make patch-<my_app_dir_name>`) and update `applications/Makefile`

### On the TomTom side
- When you boot your TomTom with OpenTom, you can directly use Telnet to login in as root from USB
- Use the built-in FTP server to update your files
- `strace` and `gdb` are ready to be used to debug your programs


## Creating Nano-X test platform on your system

- Create `$ROOT/i386` directory
- Extract, configure and install: microwin, nxlib (with libNX11), SDL, Fltk, ... (with `LDFLAGS=-L/usr/local/lib --prefix=/usr/local`)
- Try NetBeans to perform you developments?


## How to free some memory (~3-4Mo on 32!)

Use an ext2 partion on your SDcard to replace (and free) initramfs with busybox pivot_root/chroot:

- Use `fdisk` to create two partitions on your SD card: partion1=vfat(TomTom), partition2=ext2(10Mo?)
- Copy `ttsystem` into SD.part1 and verify it boots, if not try to copy gns and program directory from original TomTom and others...
- When it boots:
- Verify that busybox include chroot and pivot_root,
- Copy the unmodified `initramfs/*` into linux SD partition, with `/var/*` linked to `/tmp`
- Copy the `configs/etc_rc_ext2` to `SD.part2/etc/rc`
- Verify that kernel include ext2 filesystem support
- Copy `configs/etc_rc_file.pivot_root_ext2` to `initramfs/etc/rc`
- `make ttsystem`
- Then on partition 1 copy `build/ttsystem` and the `opentom_dist` directory on SD.part1
- If something goes wrong, try `configs/kernel_config.console_ext2` to activate kernel FrameBuffer console


## How to install gltt from TomTom `ttsystem` file (for Navit)

- Copy your TomTom™ `ttsystem` file into `$ROOT/src` (e.g. `cp /mnt/TOMTOM/ttsystem $ROOT/src/ttsystem.tomtom`)
- `cd $ROOT/src`
- `ttimgextract ttsystem.tomtom `
- `mkdir -p ttsystem.tomtom.initramfs`
- `cd ttsystem.tomtom.initramfs`
- `gunzip -c ../ttsystem.tomtom.0 | sudo cpio -i`
- Now the boot ramdisk of your TomTom is extracted to `$ROOT/src/ttsystem.tomtom.initramfs` and the TomTom kernel is in `$ROOT/src/ttsystem.1`
- Then: `cp $ROOT/src/ttsystem.tomtom.initramfs/bin/gltt $TOMDIST/bin`


## To-Do

- Fix espeak => portaudio => OSS, that currently don't work
- Patch spreadsheet to be adapted to TomTom screen


## Support

- Checkout our documentation in `docs/`
- Take a look at the shell scripts, Makefiles and patches
- Feel free to send an email in case of a problem or if you found betters URLs for sources.txt (opentom@free.fr).


## Authors

- Clément Gerardin (opentom@free.fr)
