CC=gcc
CFLAGS= 

all: mkttimage ttimgextract

.PHONY: install
install: mkttimage ttimgextract
	install -D mkttimage $(ROOT)/tools/mkttimage
	install -D ttimgextract $(ROOT)/tools/ttimgextract

mkttimage: mkttimage.o md5.o blowfish.o
	$(CC) -o mkttimage mkttimage.o md5.o blowfish.o

ttimgextract: ttimgextract.o
	$(CC) -o ttimgextract ttimgextract.o

%.o: %.c
	$(CC) -c $<

.PHONY: clean
clean:
	rm -f ttimgextract mkttimage *.o
