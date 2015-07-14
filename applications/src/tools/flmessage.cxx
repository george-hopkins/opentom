/*
 * OpenTom flmessage
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>

#define CHUNK 1024
#define ARG_STDIN 1
#define ARG_YESNO 2
#define ARG_TITLE 4
#define ARG_INPUT 8

void usage()
{
	fprintf(stderr, "Usage: xmessage [ -s ] [ -y ] [ -i ] [ -d \"default value\" ] [ -t \"title\" ] \"msg\"\n");
        exit( 1);
}

int main(int argc, char **argv)
{
int opt, flags = 0;
char *msg = 0, *def_val = 0, *title;
const char *res;
size_t len, readed = 0;

	if ( argc == 1 ) {
		usage();
	}
	while ((opt = getopt(argc, argv, "hsyt:id:")) != -1) {
		switch (opt) {
		case 's': flags |= ARG_STDIN; break;
		case 'y': flags |= ARG_YESNO; break;
		case 't': fl_message_title(optarg); break;
		case 'i': flags |= ARG_INPUT; break;
		case 'd': def_val = optarg; break;
		case 'h': usage();
		}
	}

	if ( flags & ARG_STDIN) {
		msg = (char *)malloc( CHUNK);
		while ( (len = read(0, &msg[readed], CHUNK)) == CHUNK) {
			readed += len;
			msg = (char *)realloc( msg, readed + CHUNK);
		}
	} else if( optind == argc ) usage(); else msg = argv[optind];

	if (flags & ARG_INPUT) {
		res = fl_input( msg, def_val, NULL);
		if ( res) printf("%s", res);
		return ! (res && *res);
	}

	if ( flags & ARG_YESNO ) return ! fl_ask( msg);
	else fl_message( msg);

	return 0;
}
