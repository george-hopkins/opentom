/*
 *  TomTom Go Firmware generator
 *
 *  (C) 2004 by Thomas Kleffel
 *              Matthias Kleffel
 *              Christian Daniel
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
		       

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "md5.h"
#include "blowfish.h"

// TomTom Go image magic "TTBL" -> TomTom BootLoader
unsigned char magic[4]       = {0x54, 0x54, 0x42, 0x4c};

// RAM address of root initrd image
unsigned char rootaddr[4]    = {0x00, 0x00, 0x00, 0x31};

// RAM address of kernel zImage
unsigned char kerneladdr[4]  = {0x00, 0x00, 0x70, 0x31};

// 0x00000000 as "end of images" indicator
// 0x31700000 as start address (= RAM address of zImage)
// 0x30000000 as parameter, assumedly start of RAM
unsigned char startseq[12]   = {0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x70, 0x31,
                                0x00, 0x00, 0x00, 0x30};

// Reversed TomTom Go firmware signature key
// hale to the king of arm disassemblers :-)
unsigned char ttkey[16]    =   {0xD8, 0x88, 0xd3, 0x13,
                                0xed, 0x83, 0xba, 0xad,
                                0x9c, 0xf4, 0x1b, 0x50,
                                0xb3, 0x43, 0xfa, 0xdd};

unsigned int filesize(char *name)
{
  struct stat statbuf;

  if(stat(name, &statbuf) != 0) {
    fprintf(stderr, "Cannot open file %s\n", name);
    exit(1);
  }
  return statbuf.st_size;
}

char *readfile(char *name, unsigned int size)
{
  FILE *f;
  char *buf;

  f = fopen(name, "rb");
  if(f == NULL) {
    fprintf(stderr, "Cannot open file %s\n", name);
    exit(1);
  }
  buf = (char *)malloc(size);
  if (!buf) {
	  fprintf(stderr, "error while malloc(%u)\n", size);
	  fclose(f);
	  exit(1);
  }
  if(fread(buf, 1, size, f) != size) {
    fprintf(stderr, "Error while reading file %s\n", name);
    fclose(f);
    exit(1);
  }
  fclose(f);
  return buf;
}

void sign(char *buf, unsigned int size, char *sig)
{
  struct MD5Context md5;
  BLOWFISH_CTX bf;

  MD5Init(&md5);
  MD5Update(&md5, buf, size);
  MD5Final(sig, &md5);

  Blowfish_Init(&bf, ttkey, 16);
  Blowfish_Encrypt(&bf, &((uint32_t *)sig)[0], &((uint32_t *)sig)[1]);
  Blowfish_Encrypt(&bf, &((uint32_t *)sig)[2], &((uint32_t *)sig)[3]);
}

int main(int argc, char *argv[])
{
  char *root;
  uint32_t rootsize;
  unsigned char rootsig[16];
  char *kernel;
  uint32_t kernelsize;
  unsigned char kernelsig[16];

  if(argc != 3) {
    fprintf(stderr, "Usage: %s [root.gz] [zImage]\n", argv[0]);
    return 1;
  }

  rootsize = filesize(argv[1]);
  kernelsize = filesize(argv[2]);
  root = readfile(argv[1], rootsize);
  kernel = readfile(argv[2], kernelsize);

  sign(root, rootsize, rootsig);
  sign(kernel, kernelsize, kernelsig);

  write(STDOUT_FILENO, magic, 4);

  write(STDOUT_FILENO, &rootsize, 4);
  write(STDOUT_FILENO, rootaddr, 4);
  write(STDOUT_FILENO, root, rootsize);
  write(STDOUT_FILENO, rootsig, 16);

  write(STDOUT_FILENO, &kernelsize, 4);
  write(STDOUT_FILENO, kerneladdr, 4);
  write(STDOUT_FILENO, kernel, kernelsize);
  write(STDOUT_FILENO, kernelsig, 16);

  write(STDOUT_FILENO, startseq, 12);

  return 0;
}
