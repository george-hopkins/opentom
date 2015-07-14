#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
x / 1,378125
y / 1,075


*/

#define TAILLE_BUFF 2048

int main() {
	
	char *buff;
	int x, y, w, h;
	size_t size = TAILLE_BUFF;
	char word[100], key[100], rem[500];
	
	buff = (char *)malloc(TAILLE_BUFF);
	FILE *in, *out;
	in = fopen("nxpad2-skin.txt", "r");
	out = fopen("nxpad2-skin_rescalled.txt", "w+");
	while ( getline(&buff, &size, in) > 0) {
		if ( (strncmp(buff, "map", 3) == 0) ||
		     (strncmp(buff, "cmd", 3) == 0)) {
			if (sscanf(buff, "%s %s %d %d %d %d %s", word, key, &x, &y, &w, &h, rem) >= 6) {
				printf("[%s][%s][%d][%d][%d][%d][%s]\n", word, key, x, y, w, h, rem);
			}
			strtok(buff, "#");
			fprintf( out, "%s %s %d %d %d %d # %s\n", word, key, (int)(((double)x)/1.378125),
				(int)(((double)y)/1.075), (int)(((double)w)/1.378125),
				(int)(((double)h)/1.075), strtok( NULL, "\n"));
		}
		else fprintf( out, "%s", buff);
	}
	fclose(in);
	fclose(out);
	free(buff);
	
	return 0;
}
