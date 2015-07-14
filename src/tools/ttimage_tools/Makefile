CC=gcc
CFLAGS= 

all: $(CROSS)/bin/mkttimage $(CROSS)/bin/ttimgextract

$(CROSS)/bin/mkttimage: mkttimage.o md5.o blowfish.o
	$(CC) -o $(CROSS)/bin/mkttimage mkttimage.o md5.o blowfish.o

$(CROSS)/bin/ttimgextract: ttimgextract.o
	$(CC) -o $(CROSS)/bin/ttimgextract ttimgextract.o

%.o: %.c
	$(CC) -c $<

clean:
	rm -f ttimgextract mkttimage *.o

