/*
 * OpenTom nanobrick_editor, by Clément GERARDIN
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
#include <sys/types.h>
#include <sys/wait.h>

#include "nanobrick.h"
#include "nanobrick_images.c"

#define WIN_SCORE_Hx2 (WIN_SCORE_H<<1)

char *mod_str[NB_SPECIAL_MODS] = { "None", "Bonus", "Quick", "Slow", "Tiny", "Large", "Extra Ball", "Next Level", "Wall", "Bomber", "Larger", "Smaller", "Extra live" };

sAppState app;

char **env; //, *sprites = "nanobrick/nanobrick_images.gif";

int status, sel_x, sel_y, sel_type, sel_mod, pushed, board_number;
pid_t game_pid = 0;
char level_name[200], game_name[180];
pBrick selected_brick;

void *my_malloc( size_t size) {
	void *b = malloc( size);
	if ( !b) {
		fprintf(stderr,"Out of memory !\n");
		exit(1);
	} else
		return b;
}

int load_sprites( /* char *filename */ )
{

	int x, y, n;
	GR_DRAW_ID images;
	
	images = GrNewPixmap(image_nanobrick_images.width, image_nanobrick_images.height, NULL);
	//GrDrawImageFromFile(images, app.gc, 0, 0, 100, 100, filename, 0);
	GrDrawImageBits( images, app.gc, 0, 0, &image_nanobrick_images);
		
	/* Make bricks images */
	n = 0;
	for ( x = 0; x < 2; x++)
		for ( y = 0; y < 5; y++) {
			
			app.pixmap_bricks[n] = GrNewPixmap(20, 10, NULL);
			GrCopyArea( app.pixmap_bricks[n], app.gc, 0, 0, app.level.bw, app.level.bh, images, x * 20, y * 10, 0);
			n++;
		}
	
	/* Load Ball image */
	app.pixmap_ball = GrNewPixmap(BALL_DIAM, BALL_DIAM, NULL);
	GrCopyArea( app.pixmap_ball, app.gc, 0, 0, BALL_DIAM, BALL_DIAM, images, 0, 0, 0);
	
	/* Load rac images */
	for ( y = 0; y < 5; y++) {
			app.pixmap_rac[y] = GrNewPixmap(PAD_WIDTH_MIN + (y*PAD_WIDTH_STEP), PAD_HEIGHT, NULL);
			GrCopyArea( app.pixmap_rac[y], app.gc, 0, 0, 
						PAD_WIDTH_MIN + (y*PAD_WIDTH_STEP), PAD_HEIGHT,
						images, 2 * app.level.bw, y * PAD_HEIGHT, 0);
		}
		
	/* Load bomb image */
	app.pixmap_bomb = GrNewPixmap(BOMB_WIDTH, BOMB_HEIGHT, NULL);
	GrCopyArea( app.pixmap_bomb, app.gc, 0, 0, BOMB_WIDTH, BOMB_HEIGHT, images, BALL_DIAM, 0, 0);
	
	/* Load bonus images */
	app.pixmap_bonus[0] = GrNewPixmap(BONUS_1_SIZE, BONUS_1_SIZE, NULL);
	GrCopyArea( app.pixmap_bonus[0], app.gc, 0, 0, BONUS_1_SIZE, BONUS_1_SIZE, images, 0, BALL_DIAM, 0);	
	
	app.pixmap_bonus[1] = GrNewPixmap(BONUS_2_W, BONUS_2_H, NULL);
	GrCopyArea( app.pixmap_bonus[1], app.gc, 0, 0, BONUS_2_W, BONUS_2_H, images, BALL_DIAM+BOMB_WIDTH, WALL_HEIGHT, 0);
	
	/* Load wall image */
	app.pixmap_wall = GrNewPixmap(WALL_WIDTH, WALL_HEIGHT, NULL);
	GrCopyArea( app.pixmap_wall, app.gc, 0, 0, WALL_WIDTH, WALL_HEIGHT, images, BALL_DIAM + BOMB_WIDTH, 0, 0);
	
	GrFreeImage( images);
	return 1;
}

void save_level() {
	
	FILE *f;
	int x, y;
	pLevel lv = &app.level;
	
	char next_level_name[200];
	
	sprintf( next_level_name, "%s-%d.blocks", game_name, board_number+1);
	
	f = fopen(app.level.name, "w+");	
	if ( f) {
		
		fprintf( f, "#name %s\n", app.level.name);
		fprintf( f, "#bonus %d\n", lv->bonus_level);
		fprintf( f, "#next %s\n", next_level_name);
		fprintf( f, "#dim_brick %d %d\n", 20, 10);
		fprintf( f, "#dim_tab %d %d\n", lv->sx, lv->sy);
		fprintf( f, "#bricks\n");
		
		for( y = 0; y < lv->sy; y++) {
			for( x = 0; x < lv->sx; x++) {	
					if ( lv->bricks[y][x].with_mod )
						fprintf( f, "%d:%d ", lv->bricks[y][x].type,lv->bricks[y][x].with_mod);
					else
						fprintf( f, "%d ", lv->bricks[y][x].type);
				}
			fprintf( f, "\n");			
		}
			
		fprintf( f, "\n");
		fclose(f);
	}
}

void play_level() {
	
//	char args[1024];
//	snprintf(args, 1024, "-l %s", app.level.name);
//	char *av[6] = { "nanobrick", /* "-s", sprites, */ "-l", app.level.name, 0 };
	
	if ( game_pid ) return;
	save_level();
//	fprintf(stderr, "spr=[%s]\n", sprites);
	if ( ! (game_pid=fork()) ) { execlp("nanobrick", "nanobrick", "-l", app.level.name, 0, env); perror("nanobrick"); exit(1); }
}

void set_level_size(int sx, int sy) {

	static char name[200];
	GR_WM_PROPERTIES props;
	
	if (sx < 2) sx = 2;
	if (sy < 2) sy = 2;
	if (sx >= MAX_BRICK_W) sx = MAX_BRICK_W-1;
	if (sy >= MAX_BRICK_H) sy = MAX_BRICK_H-1;

	app.level.sx = sx;
	app.level.sy = sy;
	
	app.board_height = (app.level.sy * app.level.bh) + FREE_HEIGHT;
	app.board_width = app.level.sx * app.level.bw;

	app.winW = (app.board_width>WIN_APP_W)?app.board_width:WIN_APP_W;
	app.winH = app.board_height+WIN_SCORE_Hx2;

	sprintf(name, "%s - %d - (%d x %d)", game_name, board_number, sx, sy);
	props.flags = GR_WM_FLAGS_TITLE;
	props.props = GR_WM_PROPS_APPWINDOW;
	props.title = name;
	GrSetWMProperties( app.win_app, &props);
	
	GrResizeWindow( app.win_app, app.winW, app.winH );
	GrResizeWindow( app.win_score, app.winW, WIN_SCORE_Hx2);
	GrResizeWindow( app.win_board, app.board_width, app.board_height);
	if ( app.winW > app.board_width)
		GrMoveWindow(app.win_board,app.board_x=((app.winW - app.board_width)>>1), WIN_SCORE_Hx2+1);
	else
		GrMoveWindow(app.win_board,app.board_x=0, WIN_SCORE_Hx2+1);
		
	app.pad_num = 2;
	app.pad_len = app.pad_num * PAD_WIDTH_STEP + PAD_WIDTH_MIN ;
	app.padx = (app.board_width-app.pad_len) /2;
	app.pady = app.board_height - (PAD_HEIGHT << 1);

	app.repaint = 1;
}

void load_level( int number) {

	pLevel lv;
	size_t len_buff;
	char *mod, *i, *wd, *ln;
	FILE *f;
	int m, read_brick, x, y, t;
	
	lv = &app.level;
	if ( number > 0) board_number = number;
	else board_number = 0;
	 
	/* Nettoie l'ancien level */
	app.with_wall = 0;
	app.paused = app.repaint = app.repaint_score = 1;
	
	/* Valeurs par défaut */
	lv->nb_teleports = 0;
	lv->bonus_level = 100;
	lv->bw = DEF_BRICK_W;
	lv->bh = DEF_BRICK_H;

	lv->bonus_level = 100;
	lv->nb_bricks = 0;
	
	if ( number == 0) sprintf( level_name, "%s.blocks", game_name);
	else sprintf( level_name, "%s-%d.blocks", game_name, number);
	lv->name = level_name;
	
	memset( lv->bricks, 0, sizeof(lv->bricks));
	
	ln = my_malloc(len_buff=1024);
	read_brick = y = 0;
	f = fopen(level_name, "r");	
	if ( f) {
		while ( getline(&ln, &len_buff, f) != -1) {
			if ( !read_brick)
			{
				wd = strtok(ln, " \t");

				if ( strcmp(wd,"#dim_tab") == 0) {
					sscanf(strtok(0, "\n\r"),"%d %d",&(lv->sx) ,&(lv->sy));
				}
				
				else if ( strcmp(wd,"#dim_brick") == 0)
					sscanf(strtok(0, "\n\r"),"%d %d",&(lv->bw) ,&(lv->bh));
					
				else if ( strcmp(wd,"#next") == 0)
					lv->next = strdup(strtok(0, "\n\r"));
				
				else if ( strcmp(wd,"#name") == 0)
					lv->name = strdup(strtok(0, "\n\r"));
					
				else if ( strcmp(wd,"#bonus") == 0)
					sscanf(strtok(0, "\n\r"),"%d",&(lv->bonus_level));
					
				else if ( strncmp(wd,"#bricks", 7) == 0) read_brick = 1;
				 
				else fprintf(stderr, "Load level: Unknown option %s", wd);
			} else {
				if ( ((*ln) == '\n') || ((*ln)=='\r') || ((*ln)==0)) continue;
				if ( y < MAX_BRICK_H) {
					x = 0;
					for ( i = strtok( ln, " \n\r"); i && (x < MAX_BRICK_W); i = strtok(0, " \t\n\r")) {
						
						mod = strchr( i, ':');
						if ( mod != NULL) {
							mod[0] = 0; mod++;
							m = atoi(mod);
						} else m = 0;
						
						lv->bricks[y][x].type = t = atoi(i);
						if ( (t>0) && (t<NB_BRICK_TYPES)) {
							lv->bricks[y][x].exist = (t != BRICK_TYPE_NONE);
							lv->bricks[y][x].x = x* lv->bw;
							lv->bricks[y][x].y = y* lv->bh;
							lv->bricks[y][x].with_mod = (m>=0)?(m<NB_SPECIAL_MODS)?m:0:0;
							
							if ( t == BRICK_TYPE_TELEPORT) {
								if (lv->nb_teleports < MAX_TELEPORT) {
									lv->teleports[lv->nb_teleports] = &lv->bricks[y][x];
									lv->nb_teleports++;
								} else {
									fprintf(stderr,"Load level: Toomuch teleport bricks\n");
								}
							}
							if (t && (t!=BRICK_TYPE_TELEPORT) && (t!=BRICK_TYPE_SOLID) && (t!=BRICK_TYPE_MAGIC))
									lv->nb_bricks++;
						} else 
							lv->bricks[y][x].type = lv->bricks[y][x].exist = 0;
							
						x++;
					}
					
					for ( ; x < MAX_BRICK_W; x++)
						lv->bricks[y][x].exist = lv->bricks[y][x].type = 0;
						
					y++;
				}
			}
		}
		
		/* remplissage de la fin du tableau */
		while( y < MAX_BRICK_H) {
			for ( x = 0; x < MAX_BRICK_W; x++) {
				lv->bricks[y][x].exist = lv->bricks[y][x].type = 0;
				lv->bricks[y][x].x = x* lv->bw;
				lv->bricks[y][x].y = y* lv->bh;
			}
			y++;
		}
		
		fclose(f);
	} else {
		/* Création d'un tableau par défaut */
		if ( ! number) { // si premier niveau
			lv->sx = 5 + rand() % 10;
			lv->sy = 5 + rand() % 15;
		}
		for( y = 0; y < lv->sy; y++)
			for( x = 0; x < lv->sx; x++) {					
				lv->bricks[y][x].type = lv->bricks[y][x].exist = 0;
			}
	}
	set_level_size( app.level.sx, app.level.sy);
	
	free( ln);
}

void paint_pad() {
	GrCopyArea( app.win_board, app.gc, app.padx, app.pady, 
				app.pad_len, PAD_HEIGHT, app.pixmap_rac[app.pad_num], 0, 0, 0);
}

void paint_tools() {
	static char scorestr[100];
	GrSetGCForeground( app.gc, WHITE);
	GrFillRect( app.win_score, app.gc, 0, 0, app.winW, WIN_SCORE_Hx2);
	GrSetGCForeground( app.gc, BLACK);
	sprintf( scorestr, "-           +  | x+  | x-  | y+  | y-  | P< | >N | Play it ");
	GrText( app.win_score, app.gc, 10, 10, scorestr, strlen(scorestr), 0); 
	sprintf( scorestr, " <-  |  Z  |  -> | Modifier : (%d) %s ", sel_mod, mod_str[sel_mod]);
	GrText( app.win_score, app.gc, 0, 10 + WIN_SCORE_H, scorestr, strlen(scorestr), 0); 
	
	if ( sel_type)
		GrCopyArea( app.win_score, app.gc, 20, 1, 20, 10, 
							app.pixmap_bricks[sel_type], 0, 0, 0);
	else
		GrFillRect( app.win_score, app.gc, 20, 1, 20, 10);
		
	app.repaint_score = 0;
}

void paint_all() {
	
	int x, y, t;
	char modnum[4];
	
	/* On efface tout */
	GrSetGCForeground( app.gc, BLACK);
	GrFillRect( app.win_board, app.gc, 0, 0, app.winW, app.winH);
	GrSetGCForeground( app.gc, GR_RGB(150,150,150));
	GrFillRect( app.win_board, app.gc, 0, app.level.sy * app.level.bh, app.winW, app.winH);
	
	GrSetGCForeground( app.gc, BACK_COLOR);
	GrFillRect( app.win_app, app.gc, 0, 0, app.board_x, app.winH);
	GrFillRect( app.win_app, app.gc, 0, app.board_height + WIN_SCORE_H, app.winW, app.winH - app.board_height - WIN_SCORE_H);
	GrFillRect( app.win_app, app.gc, app.board_width+app.board_x, 0, app.winW - app.board_width - app.board_x, app.winH);
	
	GrSetGCForeground( app.gc, WHITE);	
	/* Les briques */
	for( y = 0; y < MAX_BRICK_H; y++)
		for( x = 0; x < MAX_BRICK_W; x++) {
			if (app.level.bricks[y][x].exist) {
				t = app.level.bricks[y][x].type;
				GrCopyArea( app.win_board, app.gc, x * app.level.bw, y * app.level.bh,
							app.level.bw, app.level.bh, 
							app.pixmap_bricks[t], 0, 0, 0);
				if ( app.level.bricks[y][x].with_mod) {
					snprintf( modnum, 3, "%d ", app.level.bricks[y][x].with_mod); 
					GrText( app.win_board, app.gc, x * app.level.bw + 3 , y * app.level.bh + 10, modnum, 2, 0);
				}
			}
	}
	
	paint_pad();
	paint_tools();
	app.repaint = 0;
}

void event_handler (GR_EVENT *event)
{	
	pBrick pbr;
	
	switch (event->type) {
		case GR_EVENT_TYPE_CHLD_UPDATE:
				app.repaint = 1;
				break;
		case GR_EVENT_TYPE_UPDATE:
			if ( ((GR_EVENT_UPDATE *)event)->utype == GR_UPDATE_SIZE ) {
				app.winW = ((GR_EVENT_UPDATE *)event)->width;
				app.winH = ((GR_EVENT_UPDATE *)event)->height;
				app.board_x = (app.winW - app.board_width)>>1;
				if ( app.board_x < 0 ) app.board_x = 0;
				GrMoveWindow( app.win_board, app.board_x, WIN_SCORE_Hx2 + 1);
				GrResizeWindow( app.win_score, app.winW, WIN_SCORE_Hx2); 
				app.repaint = 1;
			} else break;
		case GR_EVENT_TYPE_EXPOSURE:
			app.repaint = 1;
			break;
		case GR_EVENT_TYPE_CLOSE_REQ:
			GrClose();
			exit (0);
		case GR_EVENT_TYPE_BUTTON_UP:
			if ( (event->mouse.y < WIN_SCORE_Hx2) && pushed ) {
				app.repaint_score = 1;
				if ( event->mouse.y < WIN_SCORE_H)
					switch( event->mouse.x / 20) {
						case 0: sel_type = (sel_type>0)?sel_type-1:0; break;
						case 1: break;
						case 2: sel_type = (sel_type<(NB_BRICK_TYPES-1))?sel_type+1:sel_type; break;
						case 3: set_level_size(app.level.sx+1, app.level.sy); break;
						case 4: set_level_size(app.level.sx-1, app.level.sy); break;
						case 5: set_level_size(app.level.sx, app.level.sy+1); break;
						case 6: set_level_size(app.level.sx, app.level.sy-1); break;
						case 7: save_level(); if (board_number>0) load_level( board_number-1); break;
						case 8: save_level(); load_level( board_number+1); break;
						default: play_level(); break;
					} else {
						switch( event->mouse.x / 20) {
							case 0: sel_mod = (sel_mod>0)?sel_mod-1:0; break;
							case 1: sel_mod = 0; sel_type = 0; break;
							case 2: sel_mod = (sel_mod<(NB_SPECIAL_MODS-1))?sel_mod+1:sel_mod; break;
						}
					}
			}
			selected_brick = NULL;
			pushed = 0;
			break;
		case GR_EVENT_TYPE_BUTTON_DOWN:		
			if ( event->mouse.y < WIN_SCORE_Hx2) pushed = 1;
				
		case GR_EVENT_TYPE_MOUSE_MOTION:
		
			if ( event->mouse.buttons && ( event->mouse.y > WIN_SCORE_Hx2)) {
				
				event->mouse.x -= app.board_x;
				event->mouse.y -= WIN_SCORE_Hx2;
				
				if (event->mouse.x < 0) event->mouse.x = 0;
				if (event->mouse.x >= app.board_width)
							event->mouse.x = app.board_width - 1;
					
				sel_y = event->mouse.y / app.level.bh;
				sel_x = event->mouse.x / app.level.bw;
				if ( sel_y > MAX_BRICK_H) sel_y = MAX_BRICK_H-1;
				if ( sel_x > MAX_BRICK_W) sel_x = MAX_BRICK_W-1;
				pbr = &app.level.bricks[sel_y][sel_x];
				if ( pbr != selected_brick) {
					
					pbr->with_mod = sel_mod % NB_SPECIAL_MODS;
					
					if ( sel_type || !sel_mod )
						pbr->type = sel_type % NB_BRICK_TYPES;
					
					pbr->exist = (pbr->type != BRICK_TYPE_NONE);
					app.repaint = 1;
					selected_brick = pbr;
				}
			}
			break;
		default:
			fprintf(stderr, "Got unknown event %d\n", event->type);
			break;
	}
}


int main (int argc, char**argv, char **arge)
{
	GR_EVENT ev;
	int opt;
	char *level = NULL;
	env = arge;
	
	board_number = 0;
	while ((opt = getopt(argc, argv, "l:s:n:")) != -1) {
	   switch (opt) {
	   case 'l':
		   level = optarg;
		   break;
/*	   case 's':
		   sprites = optarg;
		   break; */
	   case 'n':
			board_number = atoi(optarg);
			break;
	   default: /* '?' */
		   fprintf(stderr, "Usage: %s [-s sprite_filename] [-l game_name] [-n level_number]\n", argv[0]);
		   exit(EXIT_FAILURE);
	   }
	}
	
	if (GrOpen() < 0) {
		fprintf (stderr, "GrOpen failed");
		exit(1);
	}
	
	app.gc = GrNewGC();
	GrSetGCUseBackground (app.gc, GR_FALSE);
		
	app.win_app = GrNewWindowEx ( GR_WM_PROPS_APPFRAME | GR_WM_PROPS_CAPTION | 
								  GR_WM_PROPS_CLOSEBOX | GR_WM_PROPS_NOBACKGROUND
						 , "Nanobrick Editor",
						 GR_ROOT_WINDOW_ID, 20, 20, WIN_APP_W, WIN_APP_H, BACK_COLOR);
	GrSelectEvents ( app.win_app, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP |
								  GR_EVENT_MASK_MOUSE_MOTION | GR_EVENT_MASK_EXPOSURE |
								  GR_EVENT_MASK_UPDATE | GR_EVENT_TYPE_CHLD_UPDATE |
								  GR_EVENT_MASK_CLOSE_REQ);

	app.win_score = GrNewWindow( app.win_app, 0, 0, WIN_APP_W, WIN_SCORE_Hx2, 1, WHITE, BLACK);
	app.win_board = GrNewWindow( app.win_app, 0, 12, WIN_APP_W, WIN_APP_H, 0, WHITE, BLACK);

	if (level) strcpy(game_name, level);
	else strcpy( game_name, "No name");
	
	load_level( board_number );
	load_sprites( /* sprites */ );

	GrMapWindow (app.win_app);
	GrMapWindow (app.win_score);
	GrMapWindow (app.win_board);
	
	do {
		GrGetNextEventTimeout(&ev, 40);
		if ( game_pid ) if ( waitpid( game_pid, &status, WNOHANG) == game_pid) {
			game_pid = 0;
		}
		if ( ev.type != GR_EVENT_TYPE_TIMEOUT) event_handler(&ev);
		if ( app.repaint ) paint_all();
		if ( app.repaint_score) paint_tools();
	} while (1);
	
	return 0;
}
