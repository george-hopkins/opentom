#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>


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
Fl_Hold_Browser *browser; 

int copy_move_selected = 0, update_list=0;
pid_t running_cmd = 0;


static void fils_action (int sig) {
	int status;
	pid_t pid;

	pid = waitpid( -1, &status, WNOHANG);
	if ( pid != -1 )
		if ( pid == running_cmd) {
                    running_cmd=0;
                    update_list++;
		}
}

void load_app_ext(const char *mime_file) {
	size_t len = 0;
	char *line, *str;
	pAppExt app, *next = &first_app;
	
	FILE *f = fopen(mime_file, "r");
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

char const *add_item( const char *line, unsigned char d_type) {
	pItem *h, p = (pItem)malloc(sizeof( _sItem));
	
	p->d_type = d_type;
        p->name = strdup( line);
        p->next = 0;
	if ( ! head) head = p;
	else {
		h = &head;
                do {
                    if ( ((*h)->d_type > d_type) || (((*h)->d_type == d_type) &&  (strcmp(line,(*h)->name)<0))) {
                            p->next = *h;
                            (*h) = p;
                            break;
                            
                    } else {
                        
                        if ( (*h)->next ) h = &(*h)->next;
                        else {
                            (*h)->next = p;
                            break;
                        }
                    }
                } while ( 1);
	}

	return line;
}

void clear_item_list() {
	pItem p;
	
	browser->clear();
	while( (p = head)) {
		head = p->next;
		free(p);
	}
}


void setTitlePath(char *path) {
    static char title_buff[1024], title_buff2[1024];
    char *title, *tmp;
    int mod = 0;
        
    strncpy(title = title_buff, path, 1023);
    while ( title && strlen(title)>40) {
        if ( (tmp =strchr(title, '/'))) title = tmp+1; else break;
        mod = 1;
    }

    if ( mod) {
            snprintf(title_buff2, 1023, "%s.../%s", running_cmd?"[working] ":"", title);
    } else
            snprintf(title_buff2, 1023, "%s%s", running_cmd?"[working] ":"",path);

    win->label(title_buff2);
}

static const char *L_document_xpm[] = {
    "11 11 3 1",
    ". c None",
    "x c #d8d8f8",
    "@ c #202060",
    ".@@@@@@@@..",
    ".@xxxxx@x@.",
    ".@xxxxx@@@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@@@@@@@@@."};
static Fl_Pixmap L_documentpixmap(L_document_xpm);

static const char *L_folder_xpm[] = {
      "11 11 3 1",
      ". c None",
      "x c #d8d833",
      "@ c #808011",
      "...........",
      ".....@@@@..",
      "....@xxxx@.",
      "@@@@@xxxx@@",
      "@xxxxxxxxx@",
      "@xxxxxxxxx@",
      "@xxxxxxxxx@",
      "@xxxxxxxxx@",
      "@xxxxxxxxx@",
      "@xxxxxxxxx@",
      "@@@@@@@@@@@"};
  static Fl_Pixmap L_folderpixmap(L_folder_xpm);

  
void read_dir(char *path) {
	
	struct dirent *entree;
	DIR *rep;
        pItem item;
        int i = 1;
	
        setTitlePath( path);
        
        clear_item_list();
	rep = opendir(path);
	if ( rep == NULL) add_item(strerror(errno), 0);
	else
            while ((entree = readdir(rep)))
                    if ( strcmp(entree->d_name,"."))
                        add_item( entree->d_name, entree->d_type);     

        
        for( item =head; item; item = item->next) {
            browser->add(item->name);
            if ( item->d_type == DT_DIR)
                browser->icon(i++, &L_folderpixmap);
            else 
                browser->icon(i++, &L_documentpixmap);
        }
	
	selected = 0;
	win->redraw();
	Fl::awake();
	Fl::unlock();
}

void open_file( char *file_name)
{
	const char *ext = strrchr(file_name, '.');
	if ( ! ext) ext = "";
	
	//if ( ext) {
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
	//}
}

void run_file( char * file_name) {
	if( fork() == 0)
	{
		if(execl(file_name, file_name ,NULL) == -1){
			fprintf(stderr, "exec : %s", strerror(errno));
			exit(1);
		}
	}
}

#define CMD_QUIT 1
#define CMD_OPEN 2
#define CMD_RUN	 3
#define CMD_CUT  4
#define CMD_COPY 5
#define CMD_PASTE 6
#define CMD_DELETE 7
#define CMD_UP 8
#define CMD_RENAME 9
#define CMD_DUPLICATE 10

char copy_move_source_file[2048];

void menu_cb(Fl_Widget*, void*item) {
	const char *new_name;
	
	switch ( (int)item)
	{
	case CMD_OPEN:
		if ( ! selected) break;
		if ( selected->d_type & DT_DIR ) {
			chdir( selected->name);
			read_dir( get_current_dir_name());
		} else {
			open_file( selected->name);
			browser->select(0, 0);
		}
		break;
		
	case CMD_RUN:
		if ( selected ) run_file( selected->name);
		break;
		
	case CMD_DUPLICATE:
	case CMD_RENAME:
		if ( running_cmd || ! selected) break;
		fl_message_title("Rename ...");
		if ( (new_name = fl_input( "New name :", selected->name, NULL))) {
			if( (running_cmd = fork()) == 0) {
				if ( (int)item == CMD_RENAME)
					running_cmd = execl("/bin/mv", "/bin/mv", selected->name, new_name, NULL);
				else
					running_cmd = execl("/bin/cp", "/bin/cp", "-R", selected->name, new_name, NULL);
				perror("/bin/mv");
				exit(1);
			} else setTitlePath( get_current_dir_name());
		}
                break;
		
	case CMD_DELETE:
		if ( running_cmd || ! selected) break;
		fl_message_title("Delete ...");
		if ( fl_ask("Sure to delete '%s'%s ?", selected->name, (selected->d_type== DT_DIR)?" directory !":"")) {
			if( (running_cmd = fork()) == 0) {
				execl("/bin/rm", "/bin/rm", "-R", selected->name, NULL);
				perror("/bin/mv");
				exit(1);
			} else setTitlePath( get_current_dir_name());
		}
                break;
		
	case CMD_UP:
		chdir("..");
		read_dir( get_current_dir_name());
		break;
		
	case CMD_QUIT: exit(0); break;
	
	case CMD_COPY:
	case CMD_CUT:
                if ( selected) {
                    copy_move_selected = (int)item;
                    snprintf(copy_move_source_file, 2048, "%s/%s", get_current_dir_name(), selected->name);
                }
		break;
	
	case CMD_PASTE:
		if ( !running_cmd && copy_move_selected) {
			fl_message_title("Copy/move ...");
			if ( fl_ask("Sure to %s :\n%s\n\tinto :\n%s/\n?", 
							(copy_move_selected==CMD_COPY)?"COPY":"MOVE", copy_move_source_file, 
								get_current_dir_name())) {
									
				if( (running_cmd = fork()) == 0)
				{
					if (copy_move_selected==CMD_COPY)
						execl("/bin/cp", "/bin/cp", "-R",  copy_move_source_file, get_current_dir_name(), NULL);
					else
						execl("/bin/mv", "/bin/mv", copy_move_source_file, get_current_dir_name(), NULL);
						
					exit(1);
				} else setTitlePath( get_current_dir_name());
				if (copy_move_selected==CMD_CUT) copy_move_selected = 0;
			}
		}
		break;
		
	}
}

void browser_cb(Fl_Widget *w) {	
	Fl_Browser *b = (Fl_Browser*)w; //cast to get access to Browser methods
	// retrieve selected item from browser
	int cmd = CMD_OPEN, index = b->value();
        printf("b_cb(%p)[%d] : ev_clk=%d\n", w, b->value(), Fl::event_clicks());
	if ( index > 0) {
		selected = head;
		while (index > 1) { selected = selected->next; index--; }
                if (Fl::event_clicks()>0)
                    menu_cb( NULL, (void *)cmd);
	} else
		selected = 0;	
}
//
//void update_browser_awake_cb( void *) {
//    
//}

Fl_Menu_Item menutable[] = {
  {"goUP", 	0, menu_cb, (void*)CMD_UP, FL_MENU_DIVIDER},
  {"Edit",		0, 0, 0, FL_MENU_DIVIDER | FL_SUBMENU},
    {"Rename",  0, menu_cb, (void*)CMD_RENAME, FL_MENU_DIVIDER},
    {"Cut",     0, menu_cb, (void*)CMD_CUT},
    {"Copy",    0, menu_cb, (void*)CMD_COPY, FL_MENU_DIVIDER},
    {"Paste",   0, menu_cb, (void*)CMD_PASTE},
    {"Duplicate", 0, menu_cb, (void*)CMD_DUPLICATE, FL_MENU_DIVIDER},
    {"Delete",	0, menu_cb, (void*)CMD_DELETE},   
    {0},
  {"Open",		0, menu_cb, (void*)CMD_OPEN},
  {"Run",		0, menu_cb, (void*)CMD_RUN},
  {"   ", 	    0, 0, 0, FL_MENU_INACTIVE},
  {"Close",		0, menu_cb, (void*)CMD_QUIT},
    {0},
  {0}
};


int main(int argc, char **argv) {
	int opt;
        char *pwd;

	while ((opt = getopt(argc, argv, "c:")) != -1) {
	   switch (opt) {
	   case 'c':
			load_app_ext(optarg);
			break;
	   default: /* '?' */
		   fprintf(stderr, "Usage: %s [-c <mime file>] [directory]\n", argv[0]);
		   exit(EXIT_FAILURE);
	   }
	}

	if ( ! first_app)
		load_app_ext("/mnt/sdcard/opentom/etc/flxplorer.mime");
	
	signal( SIGCHLD, &fils_action);

	win = new Fl_Window(300,200,"FLxplorer");
	Fl_Menu_Bar menubar(0,0, 300, 20);
	menubar.menu(menutable);
	
	browser = new Fl_Hold_Browser(0,21,win->w()-1, win->h()-22, "TOTO");
        //browser->when(WHEN_RELEASE_ALWAYS);
	// browser->fl_font(FL_TIMES, 13);
	
	if ( optind < argc) read_dir(argv[optind]);
	else read_dir(get_current_dir_name());
	
	browser->callback(browser_cb);
	win->show();
        while( Fl::wait()) {
            if ( update_list) {
                read_dir( get_current_dir_name());
		//fl_message("Copy/move/remove command finished.");
		update_list = 0;
            }
        }
	return(Fl::run());
}

