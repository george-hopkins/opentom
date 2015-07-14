OpenTom SDK Readme.txt

OpenTom is a tiny OpenSource Linux distribution for TomTom(tm)

* To use this SDK :
- you need (in your system): fltk, imagemagick
- modify the ROOT env var into get_cross_env.sh
- source get_cross_env.sh
- type make to perform primary OpenTom buids

* To build (into opentom_dist) extra application
- type: make extra

Then, you will have :
- ttsystem boot image into the build directory (before copying it on TomTom SD cars, BACKUP the original ttsystem first !),
- and a opentom_dist directory (to copy on your tomtom as opentom).

** NB: For dosbox, dune2, gnuboy, linapple, and scummvm games: take it from internet and copying it into opentom/share subdirectories.


* to modify ttsystem :
- for kernel: cd kernel; make menuconfig
- for buzybox: cd build/buzibox*; make menuconfig
- for initramfs: do tou changes and touch etc/rc
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


* TODO
- make init and shutdown scripts (with pivo_root or switch_root and ext2 partition) to free 3~5 Mo of RAM (the size of the initramfs)
- make a real suspend/resume script
- patch spreadsheet to be adapted to TomTom screen


* In case of problem
- take a tour into shell scripts, Makefiles, and patchs for more informations, or at http://opentom.free.fr ... 
- Feel free to send me a mail in case of problem (opentom@free.fr).

Have fun,

Clément.

