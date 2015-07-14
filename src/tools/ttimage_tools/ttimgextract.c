/* ttimgextract - TomTom GO image extractor
 *
 * (C) 2004 by Harald Welte <laforge@ngumonks.org>
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


struct ttimg_magic {
	char		magic[4];
};

struct ttimg_sect_hdr {
	u_int32_t	sect_size;
	u_int32_t	sect_addr;
};

struct ttimg_sect_sig {
	char	signature[16];
};

static int dump_to_file(const char *pathprefix, void *start, int len)
{
	static unsigned int counter = 0;
	char fname[PATH_MAX];
	int outfd;

	sprintf(fname, "%s.%u", pathprefix, counter);

	counter++;
	
	outfd = open(fname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if (!outfd)
		return -1;

	if (write(outfd, start, len) != len) {
		close(outfd);
		return -1;
	}

	close(outfd);

	return 1;
}

static int extract(const char *pathname, int dump)
{
	int fd, ret;
	struct stat st;
	void *mem;

	struct ttimg_magic *tt_magic;
	struct ttimg_sect_hdr * tt_secthdr;

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
		goto err_out;

	if (fstat(fd, &st) < 0)
		goto err_close;


	mem = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!mem)
		goto err_close;

	tt_magic = mem;

	if (strncmp((char *)&tt_magic->magic, "TTBL", 4))
		goto err_unmap;

	for (tt_secthdr = mem + sizeof(*tt_magic);
	     (void *) tt_secthdr + sizeof(*tt_secthdr) < mem + st.st_size;
	     tt_secthdr = (void *)tt_secthdr + sizeof(*tt_secthdr)
	     		  + tt_secthdr->sect_size
			  + sizeof(struct ttimg_sect_sig)) {
		void *payload_start = (void *)tt_secthdr + sizeof(*tt_secthdr);
		struct ttimg_sect_sig *sig = payload_start + tt_secthdr->sect_size;
		unsigned int size = tt_secthdr->sect_size;
		unsigned int addr = tt_secthdr->sect_addr;

		if (size == 0)
			break;

		printf("sect_size = 0x%x, sect_addr = 0x%x\n", size, addr);

		if (dump)
			dump_to_file(pathname, payload_start, tt_secthdr->sect_size);
	}

err_unmap:
	munmap(mem, st.st_size);
err_close:
	close(fd);
err_out:
	return ret;
}

/* */
int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(2);
	}

	if (extract(argv[1], 1) < 0)
		exit(1);

	exit(0);
}
