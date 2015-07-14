#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>

typedef struct _sProcess {
	pid_t pid;
	char *info;
	struct _sProcess *next;
} *pProcess;

pProcess processToKill, head = 0;

char *add_process( const char *line) {
	pProcess h, p = (pProcess)malloc(sizeof( _sProcess));
	
	if ( strchr( line, '[') && strchr(line, ']') ) return NULL;
	
	while( ((*line) < '0') || ((*line)>'9')) line++;
	p->pid = atoi(line);
	
	if ( ! head) head = p;
	else {
		h = head;
		while ( h->next) h = h->next;
		h->next = p;
	}
	p->next = 0;
	return p->info = strdup( line);
}

void clear_process_list(Fl_Browser *browser) {
	pProcess p;
	
	while( (p = head)) {
		head = p->next;
		free(p);
		p = head;
	}
	browser->clear();
	processToKill = 0;
}

int get_ps(Fl_Browser *browser){

	pid_t pid;
	int rv;
	int commpipe[2];		/* This holds the fd for the input & output of the pipe */
	size_t buf_len;
	char *buff = 0;
	const char *str;

	/* Setup communication pipeline first */
	if(pipe(commpipe)){
		fprintf(stderr,"Pipe error!\n");
		exit(1);
	}

	/* Attempt to fork and check for errors */
	if( (pid=fork()) == -1){
		fprintf(stderr,"Fork error. Exiting.\n");  /* something went wrong */
		exit(1);        
	}

	if(pid){
		/* A positive (non-negative) PID indicates the parent process */
		dup2(commpipe[0],0);	/* Replace stdin with in side of the pipe */
		close(commpipe[1]);		/* Close unused side of pipe (out side) */
		//setvbuf(stdout,(char*)NULL,_IONBF,0);	/* Set non-buffered output on stdout */
		getline(&buff, &buf_len, stdin); // SKIPP HEADER
		
		clear_process_list(browser);
		while ( getline(&buff, &buf_len, stdin) >= 0) {
			str = add_process(buff);
			if ( str) browser->add( str);
		}
		close(commpipe[0]);
		rv = 0;
		wait(&rv);				/* Wait for child process to end */
		fprintf(stderr,"Child exited with a %d value\n",rv);
	}
	else{
		/* A zero PID indicates that this is the child process */
		dup2(commpipe[1],1);	/* Replace stdout with the out side of the pipe */
		close(commpipe[0]);		/* Close unused side of pipe (in side) */
		/* Replace the child fork with a new process */
		if(execl("/bin/ps","/bin/ps",NULL) == -1){
			fprintf(stderr,"execl Error!");
			exit(1);
		}
	}
	return 0;
}

void browser_cb(Fl_Widget *w) {	
	Fl_Browser *b = (Fl_Browser*)w; //cast to get access to Browser methods
	// retrieve selected item from browser
	int index = b->value();
	if ( index > 0) {
		processToKill = head;
		while (index > 1) { processToKill = processToKill->next; index--; }
	} else
		processToKill = 0;	
}

Fl_Browser *browser; 

void buttons_cb(Fl_Widget* buttonptr, long int userdata){

	if ( userdata && processToKill ) {
		printf("Killing %d with signal %ld\n", processToKill->pid, userdata);
		kill( processToKill->pid,userdata);
	}
	get_ps(browser);
}

int main() {
	
	Fl_Window *win = new Fl_Window(300,200,"Process Killer !");
	browser = new Fl_Browser(1,1,win->w()-1, win->h()-22);
	browser->type(FL_HOLD_BROWSER);
	get_ps( browser);
	browser->callback(browser_cb);
	Fl_Button *button0 = new Fl_Button(5,win->h()-20,50,20,"Refresh");
	Fl_Button *button1 = new Fl_Button(65,win->h()-20,40,20,"STOP");
	Fl_Button *button2 = new Fl_Button(110,win->h()-20,40,20,"CONT");
	Fl_Button *button3 = new Fl_Button(155,win->h()-20,40,20,"QUIT");
	Fl_Button *button4 = new Fl_Button(210,win->h()-20,40,20,"HUP ?");
	Fl_Button *button5 = new Fl_Button(win->w()-45,win->h()-20,40,20,"KILL !");
	button0->callback(buttons_cb, 0);
	button1->callback(buttons_cb, SIGSTOP);
	button2->callback(buttons_cb, SIGCONT);
	button3->callback(buttons_cb, SIGQUIT);
	button4->callback(buttons_cb, SIGHUP);
	button5->callback(buttons_cb, SIGKILL);
	win->show();
	return(Fl::run());
}

