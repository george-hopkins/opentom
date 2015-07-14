APP=nxtetravex

$(APP): $(APP).c
	$(CC) -o $@ $(APP).c -I$(ARM_SYSROOT)/usr/include/microwin -L$(ARM_SYSROOT)/usr/lib -lnano-X

install: $(APP)
	cp $(APP) $(TOMDIST)/bin

clean:
	rm -f *.o $(APP)
