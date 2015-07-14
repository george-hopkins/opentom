CC=gcc
CFLAGS=
LDFLAGS=


all: $(CROSS)/bin/mkttimage $(CROSS)/bin/ttimgextract

$(CROSS)/bin/mkttimage: mkttimage.o md5.o blowfish.o
	$(CC) -o $(CROSS)/bin/mkttimage mkttimage.o md5.o blowfish.o

$(CROSS)/bin/ttimgextract: ttimgextract.o 
	$(CC) -o $(CROSS)/bin/ttimgextract ttimgextract.o

clean:
	rm -f ttimgextract mkttimage *.o
