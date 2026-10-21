install: gpsdash
	cp gpsdash $(TOMDIST)/bin

gpsdash: gpsdash.c
	$(CC) -O2 -o $@ -I. -I$(ARM_APPROOT)/include/microwin gpsdash.c -L$(ARM_SYSROOT)/usr/lib -lnano-X

clean:
	rm -f gpsdash
