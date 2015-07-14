#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>

int main(int argc, char **argv) {
	int opt;
  	Fl_Native_File_Chooser fnfc;
	fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fnfc.title("Open file");

	while ((opt = getopt(argc, argv, "hdmt:e:")) != -1) {
		switch (opt) {
		case 't': fnfc.title(optarg); break;
		case 'e': fnfc.filter(optarg); break;
                case 'd': fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY); break;
                case 'm': fnfc.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE); break;
		case 'h':
		default:
			fprintf(stderr, "Usage: fl_choosefile [ -h ] [-d|m] [ -e 'suffix' ] [ -t 'window title' ]\n");
			return 1;
		} 	
	}

	if ( fnfc.show() ) return 1;
	printf("%s", fnfc.filename());
	return 0;
}
