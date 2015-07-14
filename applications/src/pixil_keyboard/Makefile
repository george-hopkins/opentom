DEBUG=-g

all: buildmap nxkeyboard

buildmap: buildmap.c bmptools.c
	gcc $(DEBUG) -o buildmap buildmap.c bmptools.c

nxkeyboard: cli.c keymap.c nxkeyboard.c srvconn.c
	$(CC) $(DEBUG) -o nxkeyboard cli.c keymap.c nxkeyboard.c srvconn.c -lnano-X

install: nxkeyboard
	cp nxkeyboard $(TOMDIST)/bin
	cp -R keymaps $(TOMDIST)/share/

clean:
	rm -f *.o nxkeyboard buildmap
