# OpenTom

[![Build Status](https://img.shields.io/circleci/project/github/george-hopkins/opentom/master.svg)](https://circleci.com/gh/george-hopkins/opentom)
[![Latest Pre-Built Image](https://img.shields.io/badge/pre--built%20image-latest-green.svg)](https://circleci.com/api/v1/project/george-hopkins/opentom/latest/artifacts/0//home/ubuntu/opentom/target/opentom.tar.gz?branch=master&filter=successful)

**OpenTom** is a tiny, open source Linux distribution for TomTom™ devices.


## Getting Started

- Install the following dependencies: subversion chrpath fluid imagemagick xsltproc
- On a 64-bit Linux host the bundled cross-compiler is a 32-bit x86 program, so 32-bit runtime libraries are needed (for example `libc6-i386` on Debian/Ubuntu)
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


## TomTom XL LIVE (4EL0) notes

Tested on a TomTom XL LIVE (model code 4EL0.017.01, serial prefix r44) booting from its internal flash. This model has no SD card slot (the slot inside the case is for a SIM card), so both `ttsystem` and the `opentom` folder go on the internal storage:

- Back up the original files first. Rename the device's `ttsystem` to `ttsystem.orig`, then copy `build/ttsystem` and the `opentom` folder to the root of the storage. To go back to the stock system, restore the original name.
- The USB gadget is CDC Ethernet (`g_ether`, device address `192.168.1.10`, `telnetd` running). On the Windows PC used for testing it could not be used directly (Device Manager error "Code 10", hardware ID `USB\COMPAT_VID_0525&Class_02&SubClass_06`); passing the device through to WSL2 with [usbipd-win](https://github.com/dorssel/usbipd-win) made it appear as a normal `cdc_ether` network interface.
- If part of the touchscreen cannot be reached (dead corners), the menu icon can be moved with `nxmenu etc/nxmenu.cfg <x> <y>` (default: top-left corner), for example in `start.sh`. A dead corner can also spoil `ts_calibrate`, whose targets are 50 px inside the screen edges.
- `ttimgextract` prints the two sections of a `ttsystem` file. Use `file` to see which output file is the gzip'd cpio ramdisk (load address 0x31000000) and which is the kernel (0x31700000). On the image tested here the ramdisk was `ttsystem.1` and the kernel `ttsystem.0`, the other way round from the gltt instructions above.

## GPS dashboard

`gpsdash` (menu: Applications / GPS Dashboard) shows speed, heading, altitude, position, fix status, satellites in use and in view, a signal bar for each satellite, battery level and UTC time, and can log the track to `logs/gps-track.csv`. Touch buttons: LOG, UNITS (mph or km/h, ft or m) and EXIT. It reads NMEA from `/var/run/gpspipe` and is started by `bin/gpsdash.sh`.

Only GPS chip type 130 ("Barracuda", see `/proc/barcelona/gpstype`) is supported so far; this is the chip of the XL LIVE. On other chips the launcher says so. Support for the NMEA chip type 3 (see `navit.sh`) and the `gltt` types 128/129 would need someone with such a device to test.

The Barracuda needs TomTom's own `glgps` program and its `glconfig.xml`. They belong to TomTom and are not included here. Both come from your own device:

- `glgps` is already on the device storage, in `bin/glgps` at the root.
- `glconfig.xml` is in the ramdisk of the original firmware. Extract your original `ttsystem` as described in the gltt section above and copy `etc/glconfig.xml` from the extracted ramdisk to `gl/cfg/glconfig.xml` on the device storage (create the folders).

Notes:

- Receivers of this generation report dates 1024 weeks too early (GPS week rollover). The dashboard adds 7168 days to any date before 2024; the time of day is not affected.
- A first fix with no stored satellite data can take several minutes under open sky, so keep the dashboard running. Closing it stops the receiver.
- The battery percentage is estimated from the battery voltage (`/dev/battery`) and is only approximate.

## To-Do

- Fix espeak => portaudio => OSS, that currently don't work
- Patch spreadsheet to be adapted to TomTom screen


## Support

- Checkout our documentation in `docs/`
- Take a look at the shell scripts, Makefiles and patches
- Feel free to send an email in case of a problem or if you found betters URLs for sources.txt (opentom@free.fr).


## Authors

- Clément Gerardin (opentom@free.fr)
