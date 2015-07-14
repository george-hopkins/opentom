#####################################################################
# OpenTom dist applications builder Makefile
#
#	version: 0.1
#
#	http://opentom.free.fr
#
######################################################################

# Check that get_build_env.sh have been sourced
ifeq ($(ROOT),)
	echo Please, do first : source get_build_env.sh
endif

TMP=/tmp
export APPS_ROOT=$(ROOT)/applications
export LOGS=$(APPS_ROOT)/logs

APPS=	tools nxmenu flxplorer pixil_keyboard nanobrick nxtetravex nxterminal
CLEAN_APPS= $(APPS:%=clean-%)

EXTRA= 	12swap ntetris pipepanic \
	madplay spreadsheet flchekers \
	yareversi opendune linapple \
	dosbox dillo xgnuboy scummvm \
	martian jooleem.sh tuxchess cr3 \
	sudoku checkers freecell

BIN_EXTRA = $(EXTRA:%=$(TOMDIST)/bin/%)

.PHONY: $(APPS)
.PHONY: $(CLEAN_APPS)

install: $(APPS)
	cp ../build/microwin/src/bin/nxmag $(TOMDIST)/bin
	cp ../build/microwin/src/bin/nxview $(TOMDIST)/bin

clean: $(CLEAN_APPS)

extra: $(BIN_EXTRA)
	@echo
	@echo "######################################################################################"
	@echo "# Build of Extra apps are done"
	@echo "# return to $$ROOT and type:"
	@echo "#"
	@echo "#       make verif_dist (to set libs and strip the all)."
	@echo "#"
	@echo "#####################################################################################"

$(APPS):
	make -C src/$@ install

$(CLEAN_APPS):
	make -C src/$(@:clean-%=%) clean


build/%/Makefile:
	if ! test -f $@; then make $(@:%/Makefile=%); fi
	cd $(@:%/Makefile=%) && ./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH);

patch-%:
	$(eval NAME :=$(@:patch-%=%))
	$(eval ONAME :=$(NAME).original)
	make Downloads/$(@:patch-%=%)*
	make -C build/$(NAME) clean || echo ok
	if ! test -d build/$(ONAME); then  cd $(TMP); { \
			if ! tar xf $(APPS_ROOT)/Downloads/$(NAME)*; then unzip $(APPS_ROOT)/Downloads/$(NAME)*; fi; \
		}; \
		mv $(TMP)/$(NAME) $(APPS_ROOT)/build/$(ONAME); \
	fi
	cd build && diff -rupN $(ONAME) $(NAME) >../patchs/$(NAME)_$(shell date +%y%m%d%k%M).patch

$(TOMDIST)/bin/freecell: Downloads/TomTom_FreeCell.zip
	mkdir -p build/freecell
	cd build/freecell && unzip ../../Downloads/TomTom_FreeCell.zip && { \
		cd src && patch -p1 <../../../patchs/freecell_opentom.patch; cd ..; \
		$(CXX) -o $(TOMDIST)/bin/freecell -D ONLY16BPP -I$(ARM_APPROOT)/include/SDL -I src/include src/*.cpp -lSDL -lpthread && \
		mkdir -p $(TOMDIST)/share/freecell; \
		cp catapps/freecell/*.bmp $(TOMDIST)/share/freecell; \
	}
		
		
$(TOMDIST)/bin/sudoku:
	cd ../build/fltk-1.3.2/test && $(CC) -I.. sudoku.cxx -lfltk -lfltk_images -o $(TOMDIST)/bin/sudoku


$(TOMDIST)/bin/checkers:
	mkdir -p build/checkers/pixmaps
	cd ../build/fltk-1.3.2/test/ && { \
		cp -R pixmaps/black*.xbm pixmaps/white*.xbm $(APPS_ROOT)/build/checkers/pixmaps; \
		cp checkers.cxx $(APPS_ROOT)/build/checkers; \
		cd $(APPS_ROOT)/build/checkers/pixmaps && mogrify -resize 50% *; \
	}
	cd build/checkers && { \
		$(CC) checkers.cxx -lfltk -lfltk_images -o $(TOMDIST)/bin/checkers; \
	}

$(TOMDIST)/bin/tuxchess:
	if ! test -d src/tuxchess; then \
		cp -R $(ROOT)/build/microwin/src/demos/tuxchess build; \
		cd build/tuxchess; \
		patch -p2 <../../patchs/nanox_tuxchess_opentom.patch; \
		mogrify -resize 23x23 images/?_*.gif; \
		rm images/board.gif; \
	fi
	cd build/tuxchess && { \
		$(CC) -o $(TOMDIST)/bin/tuxchess -I/usr/local/include/microwin *.c -lnano-X; \
		mkdir -p $(TOMDIST)/share/tuxchess && cp -R images $(TOMDIST)/share/tuxchess; \
	}
		
$(TOMDIST)/bin/12swap: Downloads/caanoo-12swap-v1.0-bin.zip Downloads/caanoo-12swap-v1.0-src.zip
	cd build && \
		unzip ../Downloads/caanoo-12swap-v1.0-bin.zip && \
		unzip -o ../Downloads/caanoo-12swap-v1.0-src.zip && \
		cd caanoo-12swap-v1.0 && { \
			patch -p1 <../../patchs/caanoo-12swap-v1.0_1402241302.patch; \
			make; \
			echo '#! /bin/sh\n\ncd $$DIST/bin/12swap\nexec ./12swap\n' >$(TOMDIST)/bin/12swap/12swap.sh; \
		}

$(TOMDIST)/bin/ntetris:
	cp $(ROOT)/build/microwin/src/bin/ntetris $(TOMDIST)/bin/ntetris

$(TOMDIST)/bin/pipepanic: Downloads/pipepanic-0.1.3-source.tar.gz
	cd build && tar xf ../Downloads/pipepanic-0.1.3-source.tar.gz && cd pipepanic-0.1.3-source && { \
		patch -p1 <../../patchs/pipepanic-0.1.3-source_opentom.patch; \
		make; \
		mkdir -p $(TOMDIST)/bin/pipepanic; \
		cp *.bmp pipepanic $(TOMDIST)/bin/pipepanic; \
		echo '#! /bin/sh\n\ncd $$DIST/bin/pipepanic\nexec ./pipepanic -320x240\n' >$(TOMDIST)/bin/pipepanic/pipe.sh; \
	}
		
$(TOMDIST)/bin/madplay: Downloads/madplay-0.15.2b.tar.gz
	cd build && tar xf ../Downloads/madplay-0.15.2b.tar.gz && cd madplay-0.15.2b && { \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) >$(LOGS)/madplay.log; \
		make $(JOBS) >$(LOGS)/madplay.log && cp madplay $(TOMDIST)/bin/madplay; \
	}

$(TOMDIST)/bin/spreadsheet: Downloads/spreadsheet1.2.2.tar.gz
	cd build && tar xf ../Downloads/spreadsheet1.2.2.tar.gz && cd spreadsheet1.2.2 && { \
		patch -p1 <../../patchs/spreadsheet1.2.2_1402241834.patch; \
		make && cp sprsht $(TOMDIST)/bin/spreadsheet; \
	}

$(TOMDIST)/bin/flchekers: Downloads/yacheckers-1.4.0.tar.bz2
	cd build && tar xf ../Downloads/yacheckers-1.4.0.tar.bz2 && cd yacheckers-1.4.0 && { \
		patch -p1 <../../patchs/yacheckers-1.4.0_1402241834.patch; \
		make -f Makefile.fl; \
		cp flchekers $(TOMDIST)/bin; \
	}

$(TOMDIST)/bin/yareversi: Downloads/yareversi-1.0.2.tar.bz2
	cd build && tar xf ../Downloads/yareversi-1.0.2.tar.bz2 && cd yareversi-1.0.2 && { \
		patch -p1 <../../patchs/yareversi-1.0.2_1402241834.patch; \
		make -f Makefile.opentom; \
		cp yareversi $(TOMDIST)/bin; \
	}

$(TOMDIST)/bin/opendune: Downloads/OpenDUNE_git.tgz
	cd build && tar xf ../Downloads/OpenDUNE_git.tgz && cd OpenDUNE && { \
		./configure --prefix=$(ARM_APPROOT) --enable-debug=0 --disable-assert --host=$(T_ARCH) --without-alsa && \
		patch -p1 <../../patchs/OpenDUNE_opentom.patch; \
		make $(JOBS) >/$(LOGS)/opendune.log 2>&1 && \
		cp bin/opendune $(TOMDIST)/bin; \
		mkdir -p $(TOMDIST)/share/dune2/data; \
		echo "# Copy Dune2 files here !" >$(TOMDIST)/share/dune2/data/README; \
	}

$(TOMDIST)/bin/linapple: Downloads/linapple-src_2a.tar.bz2 
	cd build && tar xf ../Downloads/linapple-src_2a.tar.bz2 && cd linapple-src_2a && { \
		patch -p1 <../../patchs/linapple-src_2a_1402241834.patch; \
		make >$(LOGS)/linapple.log 2>&1 && cp linapple-src_2a $(TOMDIST)/bin/linapple; \
		mkdir -p $(TOMDIST)/share/linapple-src_2a; \
		cp charset40.bmp font.bmp splash.bmp Master.dsk $(TOMDIST)/share/linapple-src_2a; \
	}

$(TOMDIST)/bin/scummvm: Downloads/scummvm-1.6.0.tar.bz2
	cd build && tar xf ../Downloads/scummvm-1.6.0.tar.bz2 && cd scummvm-1.6.0 && { \
		patch -p1 <../../patchs/scummvm-1.6.0_opentom.patch; \
		./configure --prefix=$(ARM_SYSROOT)/usr --host=$(T_ARCH) --disable-debug --default-dynamic --disable-scalers --disable-hq-scalers --disable-translation --disable-bink --with-sdl-prefix=$(ARM_SYSROOT)/usr/bin --disable-nasm --disable-readline --disable-libunity --disable-sndio --disable-engine=kyra --backend=sdl --enable-optimizations --enable-vkeybd >$(LOGS)/scummvm.log >$(LOGS)/scummvm.log; \
		VERBOSE_BUILD=1 AS=arm-linux-as make $(JOBS) >$(LOGS)/scummvm.log 2>&1; \
		make $(JOBS) >../../logs/scummvm.log && cp scummvm $(TOMDIST)/bin; \
		mkdir -p $(TOMDIST)/share/scumm_games; \
	}

$(TOMDIST)/bin/dosbox: Downloads/dosbox-0.74.tar.gz
	cd build && tar xf ../Downloads/dosbox-0.74.tar.gz && cd dosbox-0.74 && { \
		patch -p1 <../../patchs/dosbox-0.74_opentom_savesfilestate_sdl.patch; \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH) --disable-sdltest --disable-alsa-midi --disable-alsatest --disable-dynamic-core --disable-dynamic-x86 --disable-fpu-x86 --disable-opengl >$(LOGS)/dosbox.log; \
		cp ../../patchs/dosbox-0.74_config.h config.h; \
		make $(JOBS) >$(LOGS)/dosbox.log && cp src/dosbox $(TOMDIST)/bin; \
		mkdir -p $(TOMDIST)/share/dosbox_games; \
	}
		

$(TOMDIST)/bin/dillo: Downloads/dillo-3.0.3.tar.bz2
	cd build && tar xf ../Downloads/dillo-3.0.3.tar.bz2 && cd dillo-3.0.3 && { \
		./configure --prefix=$(ARM_APPROOT) --host=$(T_ARCH); \
		make $(JOBS) install >$(LOGS)/dillo.log; \
		cp -R $(ARM_APPROOT)/lib/dillo $(TOMDIST)/lib; \
		cp -R $(ARM_APPROOT)/etc/dillo $(TOMDIST)/etc; \
		cd $(ARM_APPROOT)/bin && cp dpid dillo dpidc dillo-install-hyphenation $(TOMDIST)/bin; \
	}

$(TOMDIST)/bin/mupdf: Downloads/mupdf-1.3-source.tar.gz
	cd build && tar xf ../Downloads/mupdf-1.3-source.tar.gz && cd mupdf-1.3-source && { \
		mkdir -p build/debug; \
		gcc  -Iinclude -Iscripts -Igenerated -Wall -pipe -g -DDEBUG -Ithirdparty/freetype/include -Ithirdparty/jbig2dec -Ithirdparty/jpeg -Ithirdparty/openjpeg/src/lib/openjp2  -Ithirdparty/zlib -o build/debug/cmapdump scripts/cmapdump.c; \
		gcc  -Iinclude -Iscripts -Igenerated -Wall -pipe -g -DDEBUG -Ithirdparty/freetype/include -Ithirdparty/jbig2dec -Ithirdparty/jpeg -Ithirdparty/openjpeg/src/lib/openjp2  -Ithirdparty/zlib -o build/debug/fontdump scripts/fontdump.c; \
		gcc  -Iinclude -Iscripts -Igenerated -Wall -pipe -g -DDEBUG -Ithirdparty/freetype/include -Ithirdparty/jbig2dec -Ithirdparty/jpeg -Ithirdparty/openjpeg/src/lib/openjp2  -Ithirdparty/zlib -o build/debug/bin2hex scripts/bin2hex.c; \
		CROSSCOMPILE=yes verbose=yes make prefix=$(ARM_APPROOT) install && cp build/debug/mupdf-x11 $(TOMDIST)/bin/mupdf; \
	}
		
$(TOMDIST)/bin/xgnuboy: Downloads/gnuboy-1.0.3.tar.gz
	cd build && tar xf ../Downloads/gnuboy-1.0.3.tar.gz && cd gnuboy-1.0.3 && { \
		./configure --prefix=$(ARM_SYSROOT)/usr --with-x --host=arm --without-sdl --without-fb --x-includes=$(ARM_SYSROOT)/usr/include --disable-asm --enable-optimize=full --x-libraries=$(ARM_SYSROOT)/usr/lib >$(LOGS)/gnuboy.log; \
		cat Makefile | sed 's/-lXext//' >/tmp/toto$$; mv /tmp/toto$$ Makefile; \
		make $(JOBS) >$(LOGS)/gnuboy.log && cp xgnuboy $(TOMDIST)/bin; \
		mkdir -p $(TOMDIST)/share/gnuboy/gnuboy_games; \
		cp etc/classic.rc $(TOMDIST)/share/gnuboy; \
	}
		
$(TOMDIST)/bin/martian: Downloads/martian_source.tgz
	cd build && tar xf ../Downloads/martian_source.tgz && cd Martian-Memory && { \
		patch -p1 <../../patchs/Martian-Memory_opentom.patch; \
		./configure --prefix=$(ARM_SYSROOT)/usr --build=i686-pc-linux-gnu --host=arm-linux --datadir=/mnt/sdcard/opentom/share; \
		make $(JOBS) >$(LOGS)/martian.log && { \
			cp src/martian $(TOMDIST)/bin/martian; \
			mkdir -p $(TOMDIST)/share/Martian; \
			cp -R fonts sounds gfx music data $(TOMDIST)/share/Martian; \
			echo '#! /bin/sh\n\ncd $$DIST/share/Martian\nexec martian\n' >$(TOMDIST)/bin/martian.sh; \
		} \
	}

$(TOMDIST)/bin/jooleem.sh: Downloads/jooleem_0.1.4_win32_bin.zip Downloads/jooleem_0.1.4_src.zip
	cd build && unzip ../Downloads/jooleem_0.1.4_win32_bin.zip && cd Jooleem && { \
		unzip -o ../../Downloads/jooleem_0.1.4_src.zip; \
		patch -p1 <../../patchs/Jooleem_opentom.patch; \
		make; \
		mogrify -resize 50% data/interface/*  data/marbles/*; \
		mkdir -p $(TOMDIST)/share/jooleem; \
		cp -R data jooleem $(TOMDIST)/share/jooleem; \
		echo '#! /bin/sh\n\ncd $$DIST/share/jooleem\nexec ./jooleem\n' >$(TOMDIST)/bin/jooleem.sh; \
	}

$(TOMDIST)/bin/cr3: Downloads/cr3_3.0.56.orig.tar.gz
	cd build && tar xf ../Downloads/cr3_3.0.56.orig.tar.gz && cd cr3.0.56-7 && { \
		sed 's#OPENTOM_ROOT#$(ROOT)#' <../../patchs/cr3.0.56-7_opentom_fltk.patch | patch -p1; \
		mkdir -p arm_build; \
		cd arm_build && { \
			cmake -D MAX_IMAGE_SCALE_MUL=1 -D CMAKE_TOOLCHAIN_FILE=../tools/toolchain-arm-tomtom.cmake -D GUI=CRGUI_NANOX -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=$(TOMDIST)/share/cr3 ..; \
			make && cp cr3gui/cr3 $(TOMDIST)/bin/cr3 && make install; \
		} \
	}
		

clean_duplicate_patchs:
	#
	# Type this line to remove duplicated patchs
	#
	@echo 'for a in `ls patchs/* | sort -r`; do for b in `ls patchs/* | sort -r`; do if [ "$$a" != "$$b" ]; then if diff -q "$$a" "$$b" >/dev/null 2>&1; then echo removing $$b ; rm "$$b"; break; fi fi done done'
	# To remove empty generated patchs
	@echo 'for f in patchs/*; do s=`cat $$f | wc -l`; if [ "$$s" == "0" ]; then echo removing empty patch $$f; rm $$f; fi done'
	#

clean_all:
	rm -Rf build
	make clean

Downloads/%:
	mkdir -p Downloads
	get_source.sh $(@:Downloads/%=%)

