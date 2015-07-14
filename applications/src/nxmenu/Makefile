install: nxmenu
	cp nxmenu $(TOMDIST)/bin

nxmenu: icon.c nxmenu.c
	$(CC) -g -o $@ -I. -I$(ARM_APPROOT)/include/microwin nxmenu.c -L$(ARM_SYSROOT)/usr/lib -lnano-X

icon.c: icon.bmp
	convbmp -o $@ $<

clean:
	rm -f icon.c nxmenu
