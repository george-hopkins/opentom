/*
 * OpenTom flxplorer, by Clément GERARDIN
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
 
 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>


#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>

#define MAX_APP_ARGS 10
typedef struct _sAppExt {
	int nbarg;
	char *ext, *args[MAX_APP_ARGS+1];
	struct _sAppExt *next;
} *pAppExt;


typedef struct _sItem {
	unsigned char d_type;
	char *name;
	struct _sItem *next;
} *pItem;

pAppExt first_app = 0;
pItem selected = 0, head = 0;
Fl_Window *win;
Fl_Browser *browser; 


void load_app_ext() {
	size_t len = 0;
	char *line, *str;
	pAppExt app, *next = &first_app;
	
	FILE *f = fopen("/mnt/sdcard/opentom/etc/flxplorer.mime", "r");
	if ( f) {
		while ( getline(&line, &len, f) > 0) {
			if (*line == '#') continue;
			line[strlen(line)-1]= '\0';
			
			app = (pAppExt)malloc( sizeof( _sAppExt));
			
			for( app->nbarg = -1,str = strtok(strdup(line), " \t"); str; str = strtok( NULL, " \t")) {
				if ( *str) {
					if ( app->nbarg == -1) {
						app->ext = str;
						app->nbarg =0;
					} else
						app->args[app->nbarg++] = str;
				}
						
				if ( app->nbarg >= MAX_APP_ARGS) break;
			}
			
			(*next) = app;
			next = &app->next;
		}
		app->next = 0;
		fclose(f);
		if ( len > 0) free( line);
	} else
		fprintf(stderr, "could not open mime file\n");

}

char *add_item( const char *line, unsigned char d_type) {
	pItem h, p = (pItem)malloc(sizeof( _sItem));
	char *r_name;
	
	p->d_type = d_type;
	if ( ! head) head = p;
	else {
		h = head;
		while ( h->next) h = h->next;
		h->next = p;
	}
	p->next = 0;
	p->name = strdup( line);
	
	r_name = (char *)malloc( strlen( line) + 8);
	if ( d_type & DT_DIR) strcpy( r_name, "[DIR] ");
	else strcpy( r_name, "      ");
	
	return strcat( r_name, line);
}

void clear_item_list() {
	pItem p;
	
	browser->clear();
	while( (p = head)) {
		head = p->next;
		free(p);
	}
}

int read_dir(char *path) {
	
	struct dirent *entree;
	DIR *rep;
	
	clear_item_list();
	win->label(path);
	
	rep = opendir(path);
	if ( rep == NULL) add_item(strerror(errno), 0);
	else {
		while ((entree = readdir(rep))) {
			if ( strcmp(entree->d_name,".") && strcmp(entree->d_name,".."))
				browser->add(add_item( entree->d_name, entree->d_type));
		}
		closedir(rep);
		return 1;
	}
	return 0;
}

void browser_cb(Fl_Widget *w) {	
	Fl_Browser *b = (Fl_Browser*)w; //cast to get access to Browser methods
	// retrieve selected item from browser
	int index = b->value();
	if ( index > 0) {
		selected = head;
		while (index > 1) { selected = selected->next; index--; }
	} else
		selected = 0;	
}

void open_file( char *file_name)
{
	char *ext = strrchr(file_name, '.');
	
	if ( ext) {
		printf("Openning '%s' with : ", ext);
		
		pAppExt app = first_app;
		for( app = first_app; app; app = app->next) {
			if ( (strcmp(app->ext, "*") == 0) || (strcmp(app->ext, ext) == 0)) {
				if ( fork() == 0) {
					printf("'%s' ...\n", app->args[0]);
					app->args[app->nbarg] = file_name;
					execvp( app->args[0], app->args);
					fprintf(stderr, "%s : %s", strerror(errno), app->args[0]);
					exit(1);
				}
				return;
			}
		}
	} else {
		// Running executable
		if( fork() == 0) {
			printf("Running '%s' ...\n", file_name);
			if(execl(file_name, file_name ,NULL) == -1){
				fprintf(stderr, "exec : %s", strerror(errno));
				exit(1);
			}
		}
	}
}

void buttons_cb(Fl_Widget* buttonptr, long int userdata)
{
	switch ( userdata) {
	  case 0: // Open
		if ( ! selected) break;
		printf("Openning '%s' with signal %ld\n", selected->name, userdata);
		if ( selected->d_type & DT_DIR ) {
			chdir( selected->name);
			read_dir( get_current_dir_name());
		} else {
			open_file( selected->name);
			browser->select(0, 0);
		}
		break;
	  case 1: // UP
		chdir("..");
		read_dir( get_current_dir_name());
		break;
	  case 4: // Delete
		if ( ! selected) break;
		if ( fl_ask("Delete file '%s'", selected->name)) {
			unlink(selected->name);
			read_dir( get_current_dir_name());
		}
	}
}

int main(int argc, char **argv) {
	
	load_app_ext();
	
	win = new Fl_Window(300,200,"Process Killer !");
	browser = new Fl_Browser(1,1,win->w()-1, win->h()-22);
	browser->type(FL_HOLD_BROWSER);
	// browser->fl_font(FL_TIMES, 13);
	
	if ( argc > 1) read_dir(argv[0]);
	else read_dir(get_current_dir_name());
	
	browser->callback(browser_cb);
	Fl_Button *button0 = new Fl_Button(5,win->h()-20,50,20,"Open");
	Fl_Button *button1 = new Fl_Button(65,win->h()-20,40,20,"UP");
	//Fl_Button *button2 = new Fl_Button(110,win->h()-20,40,20,"Copy");
	//Fl_Button *button3 = new Fl_Button(155,win->h()-20,40,20,"Paste");
	Fl_Button *button4 = new Fl_Button(210,win->h()-20,40,20,"Delete");
	//Fl_Button *button5 = new Fl_Button(win->w()-45,win->h()-20,40,20,"KILL !"); */
	button0->callback(buttons_cb, 0);
	button1->callback(buttons_cb, 1);
	//button2->callback(buttons_cb, 2);
	//button3->callback(buttons_cb, 3);
	button4->callback(buttons_cb, 4);
	//button5->callback(buttons_cb, 5);
	win->show();
	return(Fl::run());
}

