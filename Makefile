######################################################################
# OpenTom dist builder Makefile
#
#	version: 0.1
#
#	http://opentom.free.fr
#
######################################################################

# Sanity checks

ifeq ($(ROOT),)
    $(error Please, do first : source get_cross_env.sh)
else
    export $(ROOT)
endif

# Speedup compilation (choose 1~2 x number of CPU/Core)
export JOBS=-j6

BUSYBOX_VER=1.22.1

export LOGS=$(ROOT)/logs
export SOURCES=$(ROOT)/src
export INITRAMFS_ROOT=$(ROOT)/initramfs
export ARM_ROOT=arm-sysroot
export ARM_SYSROOT=$(ROOT)/$(ARM_ROOT)
export ARM_APPROOT=$(ARM_SYSROOT)/usr
export CONFIGS=$(ROOT)/configs
export TOMDIST=$(ROOT)/opentom_dist
export DOWNLOADS=Downloads

export ARMGCC=gcc-3.3.4_glibc-2.3.2
export CROSS=$(ROOT)/$(ARMGCC)
export T_ARCH=arm-linux
PREFIX=$(CROSS)/arm-linux/sys-root
export CFLAGS=-mlittle-endian -march=armv5te -mtune=arm9tdmi -mshort-load-bytes -fno-omit-frame-pointer -fno-optimize-sibling-calls -mno-thumb-interwork -O2 -I$(ARM_SYSROOT)/usr/include -L$(ARM_SYSROOT)/usr/lib
export CPPFLAGS=-march=armv5te -mtune=arm9tdmi -I$(PREFIX)/include -I$(PREFIX)/usr/include
LDFLAGS=-L$(PREFIX)/lib -L$(ARM_SYSROOT)/usr/lib
COMPILO=$(CROSS)/bin/$(T_ARCH)

export CC=$(T_ARCH)-gcc
export CXX=$(T_ARCH)-g++
export LD=$(T_ARCH)-ld
export NM=$(T_ARCH)-nm -B
export AR=$(T_ARCH)-ar
export AS=$(T_ARCH)-as
export RANLIB=$(T_ARCH)-ranlib
export STRIP=$(T_ARCH)-strip
export OBJCOPY=$(T_ARCH)-objcopy

export CROSS_COMPILE=$(T_ARCH)-
export ARCH=arm

# Pour les progs utilisant libtool et son problème de --sysroot, créer un script dans /usr/local/bin/$(COMPILO)-gcc
# qui contient
# #! /bin/bash
# exec ${CROSS}/bin/${COMPILO}-gcc --sysroot=${PREFIX} $*
#
# Changer PATH=/usr/local/bin:$CROSS:/usr/bin:/bin

base: tools ttsystem distrib
	@echo
	@echo "#####################################################################################"
	@echo "# Build of OpenTom Linux base done"
	@echo "#"
	@echo "# To build all others applications, type :"
	@echo "#"
	@echo "#       make extra (very long !)"
	@echo "#"
	@echo "# Else, backup your TomTom and copy it into you SDCARD :"
	@echo "#"
	@echo "#  cp build/ttsystem <dest>; cp -R $(TOMDIST) <dest>/opentom"
	@echo "#"
	@echo "#####################################################################################"
	@echo


extra: espeak libzip sdl_net ctorrent
	make -C applications extra
	make verif_dist
	@echo
	@echo "######################################################################################"
	@echo "# Build of OpenTom Linux Extra apps done"
	@echo "#"
	@echo "######################################################################################"
	@echo


ttsystem: build/ttsystem

tt2: build/initramfs.cpio.gz
	mkttimage build/initramfs.cpio.gz src/TTSystem_original/ttsystem.origTomTomXL.1 >build/ttsystem

build/ttsystem: build/initramfs.cpio.gz kernel/arch/arm/boot/zImage
	mkttimage build/initramfs.cpio.gz kernel/arch/arm/boot/zImage >build/ttsystem

build/initramfs.cpio.gz: $(CONFIGS)/initramfs_prepend kernel/arch/arm/boot/zImage initramfs/bin/busybox initramfs/etc/rc
	cp $(CONFIGS)/initramfs_prepend build/cpio_list
	rm -Rf $(INITRAMFS_ROOT)/lib/modules/*
	cd kernel && INSTALL_MOD_PATH=$(INITRAMFS_ROOT)/ make modules_install
	# 3rd pass for new sharedlibs
	install_shared_libs.sh initramfs "$(ARM_ROOT)/lib $(ARM_ROOT)/usr/lib $(CROSS)/$(T_ARCH)/lib"
	chmod u+x kernel/scripts/gen_initramfs_list.sh
	kernel/scripts/gen_initramfs_list.sh -u `id -u` -g `id -g` $(INITRAMFS_ROOT) >>build/cpio_list
	kernel/usr/gen_init_cpio build/cpio_list | gzip -9 >build/initramfs.cpio.gz

kernel/arch/arm/boot/zImage: $(ARM_ROOT) kernel/.config
	mkdir -p $(LOGS)
	cd kernel && make clean && nice -n 19 make $(JOBS) >$(LOGS)/kernel.log 2>&1

kernel/.config: $(DOWNLOADS)/golinux-tt1114405.tar.gz
	cd src && tar xf ../$(DOWNLOADS)/golinux-tt1114405.tar.gz
	ln -s src/linux-s3c24xx kernel
	cd kernel && patch -p1 <$(ROOT)/patchs/kernel_tt1114405_opentom.patch
	cp $(CONFIGS)/kernel_config.no_console kernel/.config

$(ARM_ROOT): $(ARMGCC)/lib
	mkdir -p $(ARM_ROOT)/bin
	cd $(CROSS)/$(T_ARCH)/libc/ && cp -R etc sbin lib usr $(ARM_SYSROOT)/
	#cd $(ARM_ROOT) && find . ! -type d -exec chmod a-w {} \;
	mkdir -p $(ARM_ROOT)/usr/include
	mkdir -p $(ARM_ROOT)/usr/man/man1
	chmod u+w $(ARM_ROOT)/usr/include/asm/*

$(ARMGCC)/lib: $(DOWNLOADS)/toolchain_redhat_gcc-3.3.4_glibc-2.3.2-20060131a.tar.gz
	test -d $(ARMGCC)/lib || { \
		tar xf $(DOWNLOADS)/toolchain_redhat_gcc-3.3.4_glibc-2.3.2-20060131a.tar.gz; \
		cd $(ARMGCC)/arm-linux/ && { \
			mv sys-root sys-root.orig; \
			ln -s sys-root.orig libc; \
			ln -s $(ARM_SYSROOT)/ sys-root; \
		}; \
		cd $(ROOT)/$(ARMGCC)/bin && cat $(CONFIGS)/install_links.txt | while read file; do ln -s $$file; done; \
	}

initramfs/etc/rc:
	mkdir -p initramfs/lib
	cp -Rf src/initramfs_skel/* initramfs
	cp $(ARM_SYSROOT)/lib/libnss_dns.so.2 $(ARM_SYSROOT)/lib/libnss_files.so.2 initramfs/lib
	cd initramfs/lib && find . -type f -exec $(CROSS)/bin/$(STRIP) 2>/dev/null {} \;
	install_shared_libs.sh initramfs "$(ARM_SYSROOT)/lib $(ARM_SYSROOT)/usr/lib $(CROSS)/$(T_ARCH)/lib"
	# 2nd pass for new shared libs
	install_shared_libs.sh initramfs "$(ARM_SYSROOT)/lib $(ARM_SYSROOT)/usr/lib $(CROSS)/../arm-linux/lib"
	cp $(CONFIGS)/etc_rc_file initramfs/etc/rc
	cd initramfs && ln -s etc/rc init


###############
# Busybox
###############

initramfs/bin/busybox: initramfs/etc/rc build/busybox-$(BUSYBOX_VER)/_install/bin/busybox
	cp -R build/busybox-$(BUSYBOX_VER)/_install/* initramfs

build/busybox-$(BUSYBOX_VER)/_install/bin/busybox: build/busybox-$(BUSYBOX_VER) build/busybox-$(BUSYBOX_VER)/.config
	cd build/busybox-$(BUSYBOX_VER) && { \
		make $(JOBS) >$(LOGS)/busybox.log 2>&1 && \
		make install >>$(LOGS)/busybox.log 2>&1 && \
		chrpath -d _install/bin/busybox; \
	}


build/busybox-$(BUSYBOX_VER): $(DOWNLOADS)/busybox-$(BUSYBOX_VER).tar.bz2
	mkdir -p build
	cd build && tar xf ../Downloads/busybox-$(BUSYBOX_VER).tar.bz2

build/busybox-$(BUSYBOX_VER)/.config:
	cd build/busybox-$(BUSYBOX_VER) && { \
		cp $(CONFIGS)/busybox_config.$(T_ARCH) .config; make silentoldconfig; }


#####
# Distrib
#####

distrib: libs apps verif_dist

nano-X: tslib build/microwin/src $(ARM_ROOT)/usr/include/microwin/nano-X.h
$(ARM_ROOT)/usr/include/microwin/nano-X.h: $(ARM_ROOT)/usr/include/zlib.h $(ARM_ROOT)/usr/include/jpeglib.h $(ARM_ROOT)/usr/include/freetype2/freetype/freetype.h $(ARM_ROOT)/usr/include/png.h
	cd build/microwin/src && { \
		make >$(LOGS)/nanox.log 2>&1 && \
		make install >>$(LOGS)/nanox.log 2>&1 && \
		cp bin/convb* $(ARM_APPROOT)/bin; \
	}

build/microwin/src: $(DOWNLOADS)/microwin_9ffcd17.tgz
	cd build && { \
		if ! test -d microwin; then \
			tar xf ../Downloads/microwin_9ffcd17.tgz; \
		else \
			touch microwin/src; \
		fi; \
		cd microwin && patch -p1 <$(ROOT)/patchs/microwin_git_opentom.patch; \
	}

dropbear: $(TOMDIST)/bin/dropbear
$(TOMDIST)/bin/dropbear: $(DOWNLOADS)/dropbear-2016.74.tar.bz2
	cd build && tar xf ../Downloads/dropbear-2016.74.tar.bz2 && cd dropbear* && { \
		./configure --host=arm-linux --prefix=$(ARM_APPROOT) --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-loginfunc --disable-pututline --disable-pututxline --enable-bundled-libtom --disable-syslog --disable-largefile >$(LOGS)/dropbear.log && \
		make $(JOBS) PROGRAMS="dropbear dbclient dropbearkey scp" >>$(LOGS)/dropbear.log && \
		make install PROGRAMS="dropbear dbclient dropbearkey scp" >>$(LOGS)/dropbear.log && \
		cp $(ARM_APPROOT)/bin/dbclient $(TOMDIST)/bin/ssh && \
		cp $(ARM_APPROOT)/bin/dropbearkey $(TOMDIST)/bin && \
		cp $(ARM_APPROOT)/bin/scp $(TOMDIST)/bin && \
		cp $(ARM_APPROOT)/sbin/dropbear $(TOMDIST)/bin; \
	}

dhclient: $(TOMDIST)/bin/dhclient
$(TOMDIST)/bin/dhclient: # work only in debian like dist...
	cd build && apt-get source dhcp3-client && cd dhcp3-* && { \
		./configure && make && cp work.linux-2.2/client/dhclient $(TOMDIST)/bin; \
	}


ctorrent: $(TOMDIST)/bin/ctorrent
$(TOMDIST)/bin/ctorrent: $(ARM_ROOT)/usr/include/openssl/opensslconf.h $(DOWNLOADS)/ctorrent-1.3.4.tar.bz2
	make quick-ctorrent
	cp $(ARM_APPROOT)/bin/ctorrent $(TOMDIST)/bin


################
# Libs
################

libs: nano-X sdl fltk13 extra_libs

extra_libs: sdl_mixer sdl_image sdl_ttf libmad glib1 glib2 bluez-libs curl libid3tag expat

sdl: $(ARM_ROOT)/usr/include/SDL/SDL.h
$(ARM_ROOT)/usr/include/SDL/SDL.h: $(DOWNLOADS)/SDL-1.2.15.tar.gz $(ARM_ROOT)/usr/include/microwin/nano-X.h
	cd build && tar xf ../Downloads/SDL-1.2.15.tar.gz && cd SDL-1.2.15 && { \
		patch -p1 <../../patchs/SDL-1.2.1_opentom.patch && \
		./configure --prefix=$(ARM_APPROOT) --host=arm-linux --disable-joystick --disable-cdrom --disable-alsa --disable-esd --disable-pulseaudio --disable-arts --disable-nas --disable-diskaudio --disable-mintaudio --disable-nasm --disable-altivec --disable-ipod --disable-video-x11 --disable-dga --disable-video-x11-vm --disable-video-x11-xv --disable-video-x11-xme --disable-video-x11-xrandr --disable-video-photon --disable-video-carbon --disable-cocoa --disable-ps2gs  --disable-ps3  --disable-ggi  --disable-svga  --disable-vgl  --disable-wscons --disable-video-aalib --disable-video-directfb --disable-video-caca --disable-video-qtopia --disable-video-picogui --disable-video-xbios --disable-video-gem --disable-video-opengl --disable-osmesa-shared --disable-screensaver --disable-directx --disable-atari-ldg  --enable-video-nanox --enable-nanox-share-memory --disable-video-fbcon >$(LOGS)/sdl.log; \
		make $(JOBS) install >>$(LOGS)/sdl.log 2>&1 ; \
	}

freetype2: $(ARM_ROOT)/usr/include/freetype2/freetype/freetype.h
$(ARM_ROOT)/usr/include/freetype2/freetype/freetype.h: $(DOWNLOADS)/freetype-2.3.12.tar.gz
	cd build && tar xf ../Downloads/freetype-2.3.12.tar.gz && cd freetype-2.3.12 && { \
			./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/freetype2.log 2>&1; \
			make $(JOBS) install >>$(LOGS)/freetype2.log 2>&1; \
	}

sdl_mixer: $(ARM_ROOT)/usr/include/SDL/SDL_mixer.h
$(ARM_ROOT)/usr/include/SDL/SDL_mixer.h: $(DOWNLOADS)/SDL_mixer-1.2.12.tar.gz $(ARM_ROOT)/usr/include/SDL/SDL.h
	cd build && { \
		tar xf ../Downloads/SDL_mixer-1.2.12.tar.gz && cd SDL_mixer* && { \
			./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) --disable-music-mod --disable-smpegtest --disable-music-mp3 --enable-music-mp3-mad-gpl >$(LOGS)/SDL_mixer.log >$(LOGS)/SDL_mixer.log; \
			make $(JOBS) install >>$(LOGS)/SDL_mixer.log 2>&1; \
		} \
	}

sdl_image: $(ARM_ROOT)/usr/include/SDL/SDL_image.h
$(ARM_ROOT)/usr/include/SDL/SDL_image.h: $(DOWNLOADS)/SDL_image-1.2.12.tar.gz $(ARM_ROOT)/usr/include/SDL/SDL.h
	cd build && { \
		tar xf ../Downloads/SDL_image-1.2.12.tar.gz && cd SDL_image* && { \
			patch -p1 <../../patchs/SDL_image_opentom.patch; \
			./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/SDL_images.log; \
			make $(JOBS) install >>$(LOGS)/SDL_images.log 2>&1; \
		} \
	}

sdl_ttf: $(ARM_ROOT)/usr/include/SDL/SDL_ttf.h
$(ARM_ROOT)/usr/include/SDL/SDL_ttf.h: $(DOWNLOADS)/SDL_ttf-2.0.11.tar.gz
	cd build && { \
                tar xf ../Downloads/SDL_ttf-2.0.11.tar.gz && cd SDL_ttf* && { \
                        ./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/SDL_ttf.log; \
                        make $(JOBS) install >>$(LOGS)/SDL_ttf.log; \
                } \
        }

sdl_net: $(ARM_ROOT)/usr/include/SDL/SDL_net.h
$(ARM_ROOT)/usr/include/SDL/SDL_net.h:
	make quick-SDL_net

libmad: $(ARM_ROOT)/usr/include/mad.h
$(ARM_ROOT)/usr/include/mad.h: $(DOWNLOADS)/libmad-0.15.1b.tar.gz
	cd build && { \
		tar xf ../Downloads/libmad-0.15.1b.tar.gz && cd libmad* && {
			./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/libmad.log; \
			make $(JOBS) install >>$(LOGS)/libmad.log 2>&1; \
		} \
	}

libjpeg: $(ARM_ROOT)/usr/include/jpeglib.h
$(ARM_ROOT)/usr/include/jpeglib.h: $(DOWNLOADS)/libjpeg-6b.tar.gz $(ARM_ROOT)
	cd build && tar xf ../Downloads/libjpeg-6b.tar.gz && cd libjpeg-6b && { \
		./configure --prefix=$(ARM_APPROOT) --host=arm-linux --enable-shared --enable-static >$(LOGS)/libjpeg.log; \
		make $(JOBS) install >>$(LOGS)/jpeglib.log 2>&1 ; \
	}

libpng: $(ARM_ROOT)/usr/include/png.h
$(ARM_ROOT)/usr/include/png.h: $(DOWNLOADS)/libpng-1.6.28.tar.gz
	cd build && tar xf ../Downloads/libpng-1.6.28.tar.gz && cd libpng-1.6.28 && { \
		./configure --prefix=$(ARM_APPROOT) --host=arm-linux --enable-shared --enable-static >$(LOGS)/libpng.log; \
		make $(JOBS) install >>$(LOGS)/libpng.log 2>&1 ; \
	}

zlib: $(ARM_ROOT)/usr/include/zlib.h
$(ARM_ROOT)/usr/include/zlib.h: $(DOWNLOADS)/zlib-1.2.11.tar.gz $(ARM_ROOT)
	cd build && tar xf ../Downloads/zlib-1.2.11.tar.gz && cd zlib-1.2.11 && { \
		LDSHARED="$(T_ARCH)-gcc -shared -Wl,-soname,libz.so.1" ./configure --shared --prefix=$(ARM_APPROOT) >$(LOGS)/zlib.log; \
		make $(JOBS) install >>$(LOGS)/zlib.log 2>&1 ; \
	}

tslib: $(ARM_ROOT)/usr/include/tslib.h
$(ARM_ROOT)/usr/include/tslib.h: $(ARM_ROOT) $(DOWNLOADS)/tslib-1.0.tar.bz2
	cd build && tar xf ../Downloads/tslib-1.0.tar.bz2 && cd tslib-1.0 && { \
		ac_cv_func_malloc_0_nonnull=yes ./autogen.sh >/$(LOGS)/tslib.log; \
		./configure --prefix=$(ARM_APPROOT) --host=arm-linux >>/$(LOGS)/tslib.log; \
		mv config.h temp ; grep -v rpl_malloc >config.h <temp ; rm temp; \
		make $(JOBS) install >>/$(LOGS)/tslib.log 2>&1 ; \
	}

nxlib: $(ARM_ROOT)/usr/include/X11/X.h
$(ARM_ROOT)/usr/include/X11/X.h: build/nxlib $(ARM_ROOT)/usr/include/microwin/nano-X.h
	cd build/nxlib && { \
		make $(JOBS) >$(LOGS)/nxlib.log 2>&1 && \
		make $(JOBS) install >>$(LOGS)/nxlib.log 2>&1 && \
                cp -R X11 /usr/include/X11/cursorfont.h $(ARM_SYSROOT)/usr/include; \
		cat $(CONFIGS)/x11.pc | sed 's#ARM_APPROOT#$(ARM_APPROOT)#' >$(ARM_APPROOT)/lib/pkgconfig/x11.pc; \
		cat $(CONFIGS)/xext.pc | sed 's#ARM_APPROOT#$(ARM_APPROOT)#' >$(ARM_APPROOT)/lib/pkgconfig/xext.pc; \
	}
	cd $(ARM_ROOT)/usr/lib && ln -s libX11.so libX11.so.0


build/nxlib: $(DOWNLOADS)/nxlib_7adaf0e.tgz
	cd build && { \
                if ! test -d nxlib; then \
                        tar xf ../Downloads/nxlib_7adaf0e.tgz; \
                else \
                        touch nxlib; \
		fi; \
                cd nxlib && patch -p1 <$(ROOT)/patchs/nxlib_git_opentom.patch; \
		cp -Rf /usr/include/X11 .; \
	}


fltk13: $(ARM_ROOT)/usr/include/FL/Fl.H
$(ARM_ROOT)/usr/include/FL/Fl.H: $(DOWNLOADS)/fltk-1.3.2-source.tar.gz $(ARM_ROOT)/usr/include/X11/X.h
	if ! test -x /usr/bin/fluid; then \
		echo You need Fluid on you system to build FLTK\(arm\); \
		false; \
	fi
	cp /usr/include/X11/Xlocale.h /usr/include/X11/cursorfont.h /usr/include/X11/Xmd.h $(ARM_SYSROOT)/usr/include/X11/
	cd build && { \
		if ! test -d fltk-1.3.2*; then \
			tar xf ../Downloads/fltk-1.3.2-source.tar.gz && cd fltk-1.3.2 && { \
				patch -p1 <../../patchs/fltk-1.3.2_opentom_nxlib.patch; \
				mogrify -resize 50% test/pixmaps/black*.xbm test/pixmaps/white*.xbm; \
				./configure --prefix=$(ARM_SYSROOT)/usr --host=arm-linux --x-includes=$(ARM_SYSROOT)/usr/include \
					--x-libraries=$(ARM_SYSROOT)/usr/lib --enable-shared --disable-gl --disable-xdbe \
					--disable-xft --disable-xinerama --disable-largefile --with-x >$(LOGS)/fltk13.log 2>&1; \
				sed 's/-lXext//' <makeinclude >_makeinclude; mv _makeinclude makeinclude; \
				sed 's_\.\./fluid/fluid$$(EXEEXT).-c_fluid -c_' <test/Makefile >tmp.txt; mv tmp.txt test/Makefile; \
			}; \
			cd ..; \
		fi; \
		cd fltk-1.3.2* && make $(JOBS) install >>$(LOGS)/fltk13.log 2>&1; \
		rm -f $(ARM_APPROOT)/bin/fluid; \
	}

glib1: $(ARM_ROOT)/usr/include/glib-1.2
$(ARM_ROOT)/usr/include/glib-1.2: $(DOWNLOADS)/glib-1.2.10.tar.gz
	chmod 755 $(ARM_APPROOT)/info/dir
	cd build && { \
		tar xf ../Downloads/glib-1.2.10.tar.gz && cd glib-1.2.10 && { \
			patch -p1 <../../patchs/glib-1.2.10_ready2make_arm.patch; \
			patch -p1 <../../patchs/glib-1.2.10_pretty_function.patch; \
			make $(JOBS) >$(LOGS)/glib1.log 2>&1; \
			make install >>$(LOGS)/glib1.log; \
		}; \
	}

glib2: $(ARM_ROOT)/usr/include/glib-2.0
$(ARM_ROOT)/usr/include/glib-2.0: $(DOWNLOADS)/glib-2.14.6.tar.gz
	cd build && { \
		tar xf ../Downloads/glib-2.14.6.tar.gz && cd glib-2.14.6 && { \
			cp $(CONFIGS)/glib2_config.cache_arm-linux config.cache; \
			CFLAGS="" LDFLAGS="" CXXFLAGS="" CPPFLAGS="" ./configure --prefix=$(ARM_APPROOT) --host=arm-linux --cache-file=config.cache >$(LOGS)/glib2.log && \
			make $(JOBS) install >>$(LOGS)/glib2.log; \
		}; \
	}

bluez-libs: $(ARM_ROOT)/usr/include/bluetooth/hci.h
$(ARM_ROOT)/usr/include/bluetooth/hci.h: $(DOWNLOADS)/bluez-libs-2.15-tt350126.tar.gz
	cd build && { \
		tar xf ../Downloads/bluez-libs-2.15-tt350126.tar.gz && cd bluez-libs* && { \
			./configure --prefix=$(ARM_SYSROOT)/usr --host=arm-linux >$(LOGS)/bluez-libs.log; \
			make $(JOBS) install >>$(LOGS)/bluez-libs.log; \
		} \
	}

curl: $(ARM_ROOT)/usr/include/curl/curl.h
$(ARM_ROOT)/usr/include/curl/curl.h: $(DOWNLOADS)/curl-7.51.0.tar.gz
	cd build && { \
		tar xf ../Downloads/curl-7.51.0.tar.gz && cd curl-7.51.0 && { \
			./configure --prefix=$(ARM_APPROOT) --host=arm-linux >$(LOGS)/curl.log; \
			make $(JOBS) install >>$(LOGS)/curl.log; \
		} \
	}

libid3tag: $(ARM_ROOT)/usr/include/id3tag.h
$(ARM_ROOT)/usr/include/id3tag.h: $(DOWNLOADS)/libid3tag-0.15.1b.tar.gz
	cd build && { \
                tar xf ../Downloads/libid3tag-0.15.1b.tar.gz && cd libid3tag-0.15.1b && { \
			./configure --prefix=$(ARM_APPROOT) --host=arm-linux >$(LOGS)/libid3tag.log; \
                        make $(JOBS) install >>$(LOGS)/libid3tag.log; \
                } \
        }

expat: $(ARM_ROOT)/usr/include/expat.h
$(ARM_ROOT)/usr/include/expat.h: $(DOWNLOADS)/expat-2.1.0.tar.gz
	cd build && tar xf ../Downloads/expat-2.1.0.tar.gz && cd expat-2.1.0 && { \
                ./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH); \
                make $(JOBS) install >/$(LOGS)/libexpat.log 2>&1; \
        }

espeak: $(TOMDIST)/bin/espeak
$(TOMDIST)/bin/espeak: Downloads/pa_stable_v19_20140130.tgz Downloads/espeak-1.48.02-source.zip
	cd build && tar xf ../Downloads/pa_stable_v19_20140130.tgz && cd portaudio && { \
		./configure --prefix=$(ARM_APPROOT) --host=arm-linux --without-alsa --with-alsa --without-jack --without-asihpi --without-winapi >$(LOGS)/pa_stable.log && \
		make $(JOBS) install >$(LOGS)/pa_stable.log; }
	cd build && unzip ../Downloads/espeak-1.48.02-source.zip && cd espeak-1.48* && { \
		cp src/portaudio19.h src/portaudio.h; \
		patch -p1 <../../patchs/espeak-1.48.01-source_opentom.patch; \
		make -C src $(JOBS) install >$(LOGS)/espeak.log; \
		cp $(ARM_APPROOT)/bin/espeak $(TOMDIST)/bin; \
	}

libzip: $(ARM_ROOT)/usr/include/zip.h
$(ARM_ROOT)/usr/include/zip.h: $(DOWNLOADS)/libzip-0.11.2.tar.gz
	cd build && tar xf ../Downloads/libzip-0.11.2.tar.gz && cd libzip-0.11.2 && { \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/libzip.log && \
		make $(JOBS) install >>$(LOGS)/libzip.log && \
	cp $(ARM_APPROOT)/lib/libzip/include/zipconf.h $(ARM_APPROOT)/include/; \
	}

ssl: $(ARM_ROOT)/usr/include/openssl/opensslconf.h
$(ARM_ROOT)/usr/include/openssl/opensslconf.h: Downloads/openssl-1.0.1u.tar.gz
	cd build && tar xf ../Downloads/openssl-1.0.1u.tar.gz && cd openssl-1.0.1u && { \
		CC=gcc ./Configure linux-armv4 shared --prefix=$(ARM_APPROOT) >$(LOGS)/ssl.log && \
		make >>$(LOGS)/ssl.log && \
		INSTALL_PREFIX=/mnt/sdcard/opentom make install_sw >>$(LOGS)/ssl.log; \
	}

gtk: $(ARM_ROOT)/usr/include/gtk-1.2/gtk/gtk.h
/usr/include/gtk-1.2/gtk/gtk.h: Downloads/gtk+-1.2.10.tar.gz
	cd build && tar xf ../Downloads/gtk+-1.2.10.tar.gz && cd gtk+-1.2.10 && { \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) --with-glib-prefix=$(ARM_APPROOT) && \
		make install $(JOBS) >$(LOGS)/gtk1.log; \
		sed 's#glib_libs="-L/lib#glib_libs="-L/${ARM_APPROOT}/lib#' <gtk-config | sed 's#glib_cflags="-I/include/glib-1.2 -I/lib/glib/include"#glib_cflags="-I${ARM_APPROOT}/include/glib-1.2 -I${ARM_APPROOT}/lib/glib/include"#' | sed 's#-L/usr/lib##' | sed 's#-L/lib##' >/backup/TomTom/OpenTomSDK/arm-sysroot/usr/bin/gtk-config; \
	}


################
# Apps and Tools
################

gdb: $(ARM_ROOT)/usr/bin/gdb
$(ARM_ROOT)/usr/bin/gdb: quick-gdb-7.1

tools:
	make -C src/tools install
	make $(ARMGCC)/lib


apps: $(TOMDIST) tool_apps dropbear
	make -C applications install

$(TOMDIST): nano-X
	mkdir -p $(TOMDIST)
	mkdir -p $(TOMDIST)/logs
	cp -R src/opentom_skel/* $(TOMDIST)/
	cp $(ARM_APPROOT)/bin/nano-X $(TOMDIST)/bin
	cd build/microwin/src/bin && cp nanowm setportrait nxeyes nxclock nxroach nxmag nxview slider vnc $(TOMDIST)/bin
	mkdir -p $(TOMDIST)/lib/ts/
	cp -R $(ARM_SYSROOT)/usr/lib/ts/*.so $(TOMDIST)/lib/ts/
	cp $(ARM_SYSROOT)/usr/bin/ts_calibrate $(ARM_SYSROOT)/usr/bin/ts_test  $(TOMDIST)/bin

tool_apps: csrinit bluez-utils pppd

csrinit: $(TOMDIST)/bin/csrinit
$(TOMDIST)/bin/csrinit: $(DOWNLOADS)/csrinit-tt531604.tar.gz
	cd build && { \
		tar xf ../Downloads/csrinit-tt531604.tar.gz; \
		cd csrinit && patch -p1 <../../patchs/csrinit_lowTX.patch && arm-linux-gcc -o $(TOMDIST)/bin/csrinit -DSUPPORT_USB *.c -lusb; \
	}

bluez-utils: $(ARM_ROOT)/usr/bin/rfcomm
$(ARM_ROOT)/usr/bin/rfcomm: $(DOWNLOADS)/bluez-utils-2.15.tar.gz $(ARM_ROOT)/usr/include/bluetooth/hci.h
	cd build && { \
		tar xf ../Downloads/bluez-utils-2.15.tar.gz; \
		cd bluez-utils-2.15 && { \
			./configure --prefix=$(ARM_APPROOT) --host=arm-linux; \
			make install; \
		} >$(LOGS)/bluez-utils.log 2>&1; \
	}
	cp $(ARM_APPROOT)/bin/rfcomm $(TOMDIST)/bin
	cp $(ARM_APPROOT)/sbin/hciconfig $(TOMDIST)/bin
	cp $(ARM_APPROOT)/sbin/hciattach $(TOMDIST)/bin

pppd: $(TOMDIST)/bin/pppd
$(TOMDIST)/bin/pppd: $(DOWNLOADS)/ppp-2.4.7.tar.gz
	cd build && tar xf ../Downloads/ppp-2.4.7.tar.gz && cd ppp-2.4.7 && { \
		patch -p1 <../../patchs/ppp-2.4.7_opentom.patch && \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/ppp.log && \
		make -C pppd $(JOBS) install >>$(LOGS)/ppp.log && \
		cp $(ARM_APPROOT)/sbin/pppd $(TOMDIST)/bin/pppd; \
	}

####
# Macro
####

quick-%:
	make Downloads/$(@:quick-%=%)
	cd build && { tar xf ../$(DOWNLOADS)/$(@:quick-%=%)* || unzip ../$(DOWNLOADS)/$(@:quick-%=%)*; } && cd $(@:quick-%=%)* && { \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) $(CONF_ARGS) >$(LOGS)/$@.log && \
		find . -name Makefile | while read f; do sed 's/-Wextra//' <$$f >/tmp/tmp$$$$; mv /tmp/tmp$$$$ $$f; done; \
		make >>$(LOGS)/$@.log && \
		make $(JOBS) install >>$(LOGS)/$@.log && { \
			echo "#####"; \
			echo "# Package \"$(@:quick-%=%)\" have been successfully installed in $(ARM_APPROOT)"; \
			echo "#####"; \
		} \
	}

quickb-%:
	make Downloads/$(@:quickb-%=%)
	cd build && { tar xf ../$(DOWNLOADS)/$(@:quickb-%=%)* || unzip ../$(DOWNLOADS)/$(@:quickb-%=%)*; } && cd $(@:quickb-%=%)* && { \
		./bootstrap && \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) $(CONF_ARGS) && \
		make && \
		make install; \
	}

quicka-%:
	make Downloads/$(@:quicka-%=%)
	cd build && { tar xf ../$(DOWNLOADS)/$(@:quicka-%=%)* || unzip ../$(DOWNLOADS)/$(@:quicka-%=%)*; } && cd $(@:quicka-%=%)* && { \
		./autogen.sh && \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) $(CONF_ARGS) && \
		make && \
		make install; \
	}


verif_dist:
	rm -f $(TOMDIST)/lib/* || echo ok
	libcount=0 ; while [ $$libcount -lt `find $(TOMDIST)/lib | wc -l` ] ; do \
		libcount=`find $(TOMDIST)/lib | wc -l` ; \
		install_shared_libs.sh $(TOMDIST) "$(ARM_SYSROOT)/lib $(ARM_SYSROOT)/usr/lib $(CROSS)/$(T_ARCH)/lib" ; \
	done
	cd $(TOMDIST) && find . -type f -exec $(STRIP) 2>/dev/null {} \;


extract_initramfs:
	mkdir -p /tmp/initramfs
	cd /tmp/initramfs && { sudo rm -Rf /tmp/initramfs/*; gunzip -c $(ROOT)/build/initramfs.cpio.gz | sudo cpio -i; }

clean_all:
	make -C applications clean_all
	cd kernel && make mrproper
	rm -Rf build initramfs arm-sysroot $(TOMDIST)
	rm -f $(LOGS)/*


Downloads/%:
	mkdir -p Downloads
	get_source.sh $*

help:
	@echo "Usage: make <target>"
	@echo ""
	@echo "Where <target> can be :"
	@echo `grep : Makefile  | cut -f1 -d: | grep -v \( | grep -v build | grep -v \# | grep -v kernel | grep -v echo | grep -v initramfs | xargs`
