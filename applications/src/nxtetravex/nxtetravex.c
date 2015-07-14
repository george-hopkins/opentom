/*
 * OpenTom NXTetravex, by Clément GERARDIN
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <nano-X.h>

#define APP_WIN_H	215
#define APP_WIN_W	310
#define BORDER		10
#define MIN_TILE_SIZE 30

#define APP_STATE_NOGAME	0
#define APP_STATE_INGAME	1
#define APP_STATE_ENDOFGAME	2

#define BLACK		GR_RGB(0,0,0)
#define WHITE 		GR_RGB(255,255,255)
#define GRAY 	    GR_RGB(128,128,128)
#define DARK		GR_RGB(70,70,70)
#define BOARD_COLOR GR_RGB(0,128,0)
#define BOARD_CELL  GR_RGB(0,100,0)

#define GRID(x,y)	(app->grid[(x)+(y)*app->grid_dim])

GR_COLOR tiles_colors[10] = { GR_RGB(0,0,0), 
						GR_RGB(220,54,167), GR_RGB(255, 0, 0), GR_RGB(195, 133, 0),
						GR_RGB(255,255,0), GR_RGB(0, 255, 0), GR_RGB(0, 72, 255),	
						GR_RGB(180,0,255), GR_RGB(128, 128, 128), GR_RGB(250, 250, 250)};
						
int color_numbers[10] = { WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, WHITE, WHITE, WHITE, BLACK };

typedef struct _sTile {
	int x, y;	/* position */
	int top, right, bottom, left; /* numbers */
	struct _sTile *next;
} sTile, *pTiles;

typedef struct _sAppState {
	
	GR_WINDOW_ID app_wid;
	GR_COORD winH, winW; /* height and width of main window */
	GR_GC_ID gc;
	GR_IMAGE_ID offscreen;
	
	int state, grid_dim;
	pTiles *grid, on_floor, moved;
	int mov_dx, mov_dy, mov_from_x, mov_from_y, mov_from_grid;
	
	int tile_dim, num_w, num_h;
	int quit, repaint, repaint_back, repaint_clock, game_time, best_time[4];
} sAppState, *pAppState;

pAppState app;


void *my_malloc( size_t size) {
	void *b = malloc( size);
	if ( !b) {
		fprintf(stderr,"Out of memory !\n");
		exit(1);
	} else
		return b;
}

static void alarm_handler(int sig) {

	if ( app->state == APP_STATE_INGAME) {
		app->game_time++;
		app->repaint_clock = 1;
	}
	alarm(1);
}


void recalculate_tile_size() {
	
	int x = (app->winH > (app->winW/2)) ? app->winW / 2 : app->winH;
	x = (x - 2 * BORDER) / app->grid_dim;
	app->tile_dim = (x<=MIN_TILE_SIZE)?MIN_TILE_SIZE:x;
	
	if ( app->offscreen > 0) GrFreeImage( app->offscreen);
	app->offscreen = 0;
}

void set_grid_size(int new_dim) {
	int i;
	
	for ( i = app->grid_dim * app->grid_dim - 1; i>=0; i--)
		if ( app->grid[i] ) {
			app->grid[i]->next = app->on_floor;
			app->on_floor = app->grid[i];
		}
		
	if (app->grid_dim != new_dim) {
		
		if ( app->grid_dim != 0 ) free(app->grid);
		app->grid_dim = new_dim;
		app->grid = my_malloc(sizeof(pTiles)*app->grid_dim*app->grid_dim);		
		recalculate_tile_size();
	}
	for( i = app->grid_dim * app->grid_dim; i; i--) app->grid[i-1] = NULL;
	app->repaint = 1;
}

void clear_tiles_on_floor() {
	pTiles t = app->on_floor;
	
	while ( app->on_floor != NULL) {
		t = app->on_floor->next;
		free( app->on_floor);
		app->on_floor = t;
	}
}

void renew_game_set(int new_dim) {
	pTiles t;
	int x, y, n, size;
	
	/* Clear old set */
	set_grid_size( new_dim);
	clear_tiles_on_floor();
	
	for ( x = 0; x < app->grid_dim; x++)
		for ( y = 0; y < app->grid_dim; y++) {
			t = my_malloc(sizeof(sTile));
			
			if ( x == 0) t->left = rand()%10;
			else t->left = GRID(x-1,y)->right;
			
			if ( y == 0) t->top = rand()%10;
			else t->top = GRID(x,y-1)->bottom;
			
			t->right = rand()%10;
			t->bottom = rand()%10;
			GRID(x,y) = t;
		}
		
	n = size = app->grid_dim*app->grid_dim;
	while ( n) {
		x = rand()%size;
		t = app->grid[x];
		if ( t) {
			t->x = 2*BORDER+app->grid_dim*app->tile_dim + rand()%100;
			t->y = BORDER + rand() % (app->winH - app->tile_dim - BORDER * 2);
			t->next = app->on_floor;
			app->on_floor = t;
			app->grid[x] = NULL;
			n--;
		}
	}
	app->state = APP_STATE_INGAME;
	app->repaint = 1;
}
	
void paint_tile( GR_WINDOW_ID wid, pTiles it) {

	GR_POINT pointtable[3];
	char str[2];
	
	str[1] = 0;
	
	/* TOP */
	pointtable[0].x =it->x;
	pointtable[0].y =it->y;
	pointtable[1].x =it->x + app->tile_dim / 2;
	pointtable[1].y =it->y + app->tile_dim / 2;	
	pointtable[2].x =it->x + app->tile_dim;
	pointtable[2].y =it->y;
	GrSetGCForeground( app->gc, tiles_colors[it->top]);
	GrFillPoly( wid, app->gc, 3, pointtable);

	/* LEFT */
	pointtable[2].x =it->x;
	pointtable[2].y =it->y + app->tile_dim;	
	GrSetGCForeground( app->gc, tiles_colors[it->left]);
	GrFillPoly( wid, app->gc, 3, pointtable);	
	
	/* BOTTOM */
	pointtable[0].x =it->x + app->tile_dim;
	pointtable[0].y =it->y + app->tile_dim;
	GrSetGCForeground( app->gc, tiles_colors[it->bottom]);
	GrFillPoly( wid, app->gc, 3, pointtable);
	
	/* RIGHT */
	pointtable[2].x =it->x + app->tile_dim;
	pointtable[2].y =it->y;	
	GrSetGCForeground( app->gc, tiles_colors[it->right]);
	GrFillPoly( wid, app->gc, 3, pointtable);		
	
	/* TEXTs */
	GrSetGCForeground( app->gc, color_numbers[it->top]);
	str[0] = '0' + it->top;
	GrText( wid, app->gc, it->x+(app->tile_dim-app->num_w)/2, it->y + app->num_h, str, 1, 0);

	GrSetGCForeground( app->gc, color_numbers[it->bottom]);
	str[0] = '0' + it->bottom;
	GrText( wid, app->gc, it->x+(app->tile_dim-app->num_w)/2, it->y + app->tile_dim - 2, str, 1, 0);

	GrSetGCForeground( app->gc, color_numbers[it->left]);
	str[0] = '0' + it->left;
	GrText( wid, app->gc, it->x + (app->tile_dim/2-app->num_w)/2 - 2, it->y + (app->tile_dim + app->num_h)/2, str, 1, 0);

	GrSetGCForeground( app->gc, color_numbers[it->right]);
	str[0] = '0' + it->right;
	GrText( wid, app->gc, it->x+(app->tile_dim-app->num_w)-2, it->y + (app->tile_dim + app->num_h)/2, str, 1, 0);
	
	GrSetGCForeground( app->gc, DARK);
	GrLine( wid, app->gc, it->x, it->y, it->x + app->tile_dim, it->y + app->tile_dim);
	GrLine( wid, app->gc, it->x, it->y + app->tile_dim, it->x + app->tile_dim, it->y);
	GrRect( wid, app->gc, it->x-1, it->y-1, app->tile_dim +2, app->tile_dim +2);
}

void paint_clock() {
	char time_str[8];
	
	GrSetGCForeground( app->gc, BOARD_COLOR);
	GrFillRect( app->app_wid, app->gc, BORDER - 1, BORDER + app->grid_dim * app->tile_dim + 9, app->num_w * 7, app->num_h+1);
	GrSetGCForeground( app->gc, WHITE);
	snprintf(time_str, 8, "%02d:%02d", app->game_time/60, app->game_time%60);
	GrText( app->app_wid, app->gc, BORDER, BORDER + app->grid_dim * app->tile_dim + 10 + app->num_h, time_str, strlen(time_str), 0);
	app->repaint_clock = 0;
}

void paint_game() {
	int n, x, y, d;
	pTiles t;
	char text[2];
	
	if ( ! app->offscreen) {
		app->offscreen = GrNewPixmap( app->winW, app->winH, NULL);
		app->repaint_back = 1;
	}
	
	if ( app->repaint_back) {
		GrSetGCForeground( app->gc, BOARD_COLOR);
		GrFillRect(app->offscreen, app->gc, 0, 0, app->winW, app->winH);
		
		text[1] = n = 0; d = app->tile_dim + 2;
		for ( y = 0; y < app->grid_dim; y++)
			for(x = 0; x < app->grid_dim; x++)
				if ( GRID(x,y) == NULL) {
					GrSetGCForeground( app->gc, BOARD_CELL);
					GrFillRect( app->offscreen, app->gc, BORDER + d * x,
							BORDER + d * y, app->tile_dim, app->tile_dim); 
					if ( (app->state == APP_STATE_NOGAME) && (n<4) ) {
						GrSetGCForeground( app->gc, WHITE);
						text[0] = '2' + n++;
						GrText( app->offscreen, app->gc, BORDER + d * x + 10, BORDER + d * y + app->num_h + 10, text, 1 ,0);
					}
				}
				else paint_tile( app->offscreen, GRID(x,y));

		for( t = app->on_floor; t; t = t->next)			
			paint_tile( app->offscreen, t);
			
		app->repaint_back = 0;
	}
	GrCopyArea( app->app_wid, app->gc, 0, 0, app->winW, app->winH, app->offscreen, 0, 0, 0); 
	
	if ( app->moved != NULL)
		paint_tile( app->app_wid, app->moved);
		
	if ( app->state != APP_STATE_NOGAME ) paint_clock();
	
	if ( app->state == APP_STATE_ENDOFGAME ) {
		GrSetGCForeground( app->gc, WHITE);
		GrText( app->app_wid, app->gc, BORDER*2 + app->grid_dim*app->tile_dim, app->winH/2, "Finished !!!", 12, 0);
	}
	
	app->repaint = 0;
}

void get_tile_at( int x, int y) {

	int i;
	pTiles t2, t;
	
	app->mov_from_grid=-1;
	for ( i = app->grid_dim * app->grid_dim; i > 0; i--) {
			t = app->grid[i-1];
			if ( t && (t->x <= x) && (t->y <= y) && (t->x+app->tile_dim>=x) && (t->y+app->tile_dim >= y)) {
				app->grid[i-1] = NULL;
				app->mov_from_grid=i-1;
				break;
			}
			t = NULL;
	}
	
	if ( ! t) {		
		t = app->on_floor;
		t2 = NULL;
		while ( t ) {
			if ( (t->x <= x) && (t->y <= y) && (t->x+app->tile_dim>=x) && (t->y+app->tile_dim >= y))
				t2 = t;
			t = t->next;
		}
		t = t2;
	
		if ( t) {
			t2 = app->on_floor;
			if ( t2 == t) app->on_floor = t->next;
			else {
				while ( t2->next != t) t2 = t2->next;
				t2->next = t->next;
			}
		}
	}
		
	app->moved = t;
	if ( t) {
		app->mov_from_x = t->x;
		app->mov_from_y = t->y;
		app->mov_dx = x - t->x;
		app->mov_dy = y - t->y;
		t->next = NULL;
	}
}

int verif_set( int bx, int by, pTiles t) {
	
	if ( (bx > 0) && GRID(bx-1,by) && (GRID(bx-1,by)->right != t->left)) return 0;
	if ( (bx < app->grid_dim-1) && GRID(bx+1,by) && (GRID(bx+1,by)->left != t->right)) return 0;
	if ( (by > 0) && GRID(bx,by-1) && (GRID(bx,by-1)->bottom != t->top)) return 0;
	if ( (by < app->grid_dim-1) && GRID(bx,by+1) && (GRID(bx,by+1)->top != t->bottom)) return 0;
	return 1;
}
 
void put_tile() {
	int n, tx, ty, xo, yo, bad;
	pTiles t;
	
	bad = 0;
	if ( ! app->moved) return;
	
	xo = app->moved->x + app->tile_dim/2;
	yo = app->moved->y + app->tile_dim/2;	
	tx = (xo - BORDER) / (app->tile_dim + 2);
	ty = (yo - BORDER) / (app->tile_dim + 2);

	if ( (tx>=0) && (ty>=0) && (tx<app->grid_dim) && (ty<app->grid_dim)) {
		n = tx + ty * app->grid_dim;
		if ( (app->grid[n] == NULL) && verif_set(tx,ty,app->moved) ){
			app->moved->x = tx * (app->tile_dim+2) + BORDER;
			app->moved->y = ty * (app->tile_dim+2) + BORDER;
			app->grid[n] = app->moved;
			app->moved = NULL;			
			return;
		} else bad = 1;
	}
		
	if ( bad) {
		app->moved->x = app->mov_from_x;
		app->moved->y = app->mov_from_y;
		if ( app->mov_from_grid != -1 ) {
			app->grid[app->mov_from_grid]=app->moved;
			app->moved = NULL;
			app->repaint = 1;
			return;
		}
	}
	if ( app->on_floor) {
		for ( t = app->on_floor; t->next; t = t->next);
		t->next = app->moved;
	} else
		app->on_floor = app->moved;
		
	app->moved = NULL;
	app->repaint = 1;
}

int verif_end_of_game() {

	int n = app->grid_dim*app->grid_dim;

	while ( n) if (app->grid[--n]==NULL) return 0;
	app->state = APP_STATE_ENDOFGAME;
	app->repaint = 1;
	return 1;
}

void init_game() {
	GR_SIZE i;
	
	srand ( time(NULL) );
	
	app = my_malloc(sizeof(sAppState));
	memset( app, 0, sizeof( sAppState));
		
	app->gc = GrNewGC();
	GrSetGCUseBackground (app->gc, GR_FALSE);
	
	app->app_wid = GrNewWindowEx ( 
	       GR_WM_PROPS_APPFRAME | GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX
			 | GR_WM_PROPS_NOBACKGROUND, "NX Tetravex",
						 GR_ROOT_WINDOW_ID, 20, 20, app->winW = APP_WIN_W, app->winH = APP_WIN_H, WHITE);

	GrSelectEvents (app->app_wid, GR_EVENT_MASK_BUTTON_DOWN | 
							 GR_EVENT_MASK_BUTTON_UP |
							 GR_EVENT_MASK_MOUSE_POSITION |
							 GR_EVENT_MASK_EXPOSURE |
							 GR_EVENT_MASK_UPDATE |
							 GR_EVENT_MASK_CLOSE_REQ);
	
	GrGetGCTextSize( app->gc, "8", 1, 0, &app->num_w, &i, &app->num_h);

	set_grid_size(2);
	
	signal( SIGALRM, &alarm_handler);
	alarm(1);
	GrMapWindow (app->app_wid);
}


int main (int argc, char**argv)
{
	int x, y;
	GR_EVENT *event = my_malloc(sizeof(GR_EVENT));
	
	if (GrOpen() < 0) exit(1);
	
	init_game();

	do {
		
		GrGetNextEventTimeout(event, 40);
		if ( event->type == GR_EVENT_TYPE_TIMEOUT) continue;

		switch (event->type) {
			case GR_EVENT_TYPE_UPDATE:
				if ( ((GR_EVENT_UPDATE *)event)->utype == GR_UPDATE_SIZE ) {
					app->winW = ((GR_EVENT_UPDATE *)event)->width;
					app->winH = ((GR_EVENT_UPDATE *)event)->height;
					recalculate_tile_size();
				} else break;
				
			case GR_EVENT_TYPE_EXPOSURE:
				app->repaint = 1;
				break;
				
			case GR_EVENT_TYPE_CLOSE_REQ:
				GrClose();
				exit (0);
				
			case GR_EVENT_TYPE_BUTTON_UP:
					switch ( app->state ) {
						case APP_STATE_INGAME:
							if ( app->moved) {
								put_tile();
								app->repaint = app->repaint_back = 1;
								verif_end_of_game();
							}
							break;
						case APP_STATE_ENDOFGAME:
							set_grid_size(2);
							clear_tiles_on_floor();
							app->state = APP_STATE_NOGAME;
							app->repaint = app->repaint_back = 1;
							break;
						case APP_STATE_NOGAME:
							app->game_time = 0;
							x= (event->mouse.x - BORDER) / app->tile_dim;
							y= (event->mouse.y - BORDER) / app->tile_dim;
							if ( x>=0 && y >=0 && x<2 && y<2)
								renew_game_set(2 + x + (2*y));	
							app->repaint = app->repaint_back = 1;
					}
					break;
	
			case GR_EVENT_TYPE_BUTTON_DOWN:	
					if ( app->state == APP_STATE_INGAME ) {
							get_tile_at(event->mouse.x, event->mouse.y);
							if (app->moved != NULL)
								app->repaint = app->repaint_back = 1;
					}
					break;
				
			case GR_EVENT_TYPE_MOUSE_POSITION:
					if ( app->moved) {
						app->moved->x = event->mouse.x - app->mov_dx;
						app->moved->y = event->mouse.y - app->mov_dy;
						app->repaint = 1;
					}
					break;
		}
		if ( app->repaint ) paint_game();
		if ( app->repaint_clock ) paint_clock();
		
	} while ( ! app->quit);
	
	GrUnmapWindow(app->app_wid);
	GrDestroyWindow(app->app_wid);
	GrDestroyGC(app->gc);
	GrClose();
	return 0;
}

