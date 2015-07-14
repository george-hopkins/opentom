/*
 * OpenTom nanobrick, by Clément GERARDIN
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

#include "nanobrick.h"
#include "nanobrick_images.c"

sAppState app;

/* Exemple de fichier tableau
#name Le premier
#next tab.levelfic
#bonus 1000
#tab_dim 10 10
#brick_dim 20 10
#bricks
0 0 0 0 0 0 0 0 0 0 
1 1 1 1 1 1 1 1 1 1 
1 1 0 5 1 1 1 0 1 1
2 1 0 1 2 2 1 0 5 2
1 2 3 4 4 4 4 3 2 1
*/

void *my_malloc( size_t size) {
	void *b = malloc( size);
	if ( !b) {
		fprintf(stderr,"Out of memory !\n");
		exit(1);
	} else
		return b;
}

void load_level( char *gamename) {

	pLevel lv;
	size_t len_buff;
	char *mod, *i, *wd, *ln, filename[128];
	FILE *f;
	int m, read_brick, x, y, t;
	GR_WM_PROPERTIES props;
	
	lv = &app.level;
	 
	if ( gamename) {
		if ( strstr(gamename, ".blocks")) strncpy(filename,gamename, 128); 
		else snprintf( filename, 128, "%s.blocks", gamename);
	}
	
	/* Nettoie l'ancien level */
	app.with_wall = 0;
	app.paused = app.repaint = app.repaint_score = 1;
	while ( app.ballList) del_ball( app.ballList);
	while ( app.bombList) del_bomb( app.bombList);
	while ( app.bonusList) del_bonus( app.bonusList);
	while ( app.activatedModeList) del_mod( app.activatedModeList, 0);
	memset( lv->bricks, 0, sizeof(lv->bricks));
		
	/* Valeurs par défaut */
	lv->nb_teleports = 0;
	lv->bonus_level = 100;
	lv->bw = DEF_BRICK_W;
	lv->bh = DEF_BRICK_H;
	lv->sx = 10;
	lv->sy = 20; /* dimension du tableau de briques */
	lv->bonus_level = 100;
	lv->name = "No name";
	lv->nb_bricks = 0;
	
	ln = my_malloc(len_buff=1024);
	read_brick = y = 0;
	if ( gamename) f = fopen(filename, "r"); else f = NULL;
	if ( f) {
		while ( getline(&ln, &len_buff, f) != -1) {
			if ( !read_brick)
			{
				wd = strtok(ln, " \t");

				if ( strcmp(wd,"#dim_tab") == 0) {
					sscanf(strtok(0, "\n\r"),"%d %d",&(lv->sx) ,&(lv->sy));
					printf("sx=%d sy=%d\n", lv->sx, lv->sy);
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
				 
				else fprintf(stderr, "Unknown option %s", wd);
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
									fprintf(stderr, "Toomuch teleport bricks (max %d)\n", MAX_TELEPORT);
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
		lv->name = "Random";
		lv->sx = 5 + rand() % 10;
		lv->sy = 5 + rand() % 10;
		for( y = rand()%3; y < lv->sy; y++)
			for( x = 0; x < lv->sx; x++) {
							
				if((lv->bricks[y][x].type = 1+rand()%4)!=0) {
					lv->bricks[y][x].exist = 1;
					if ( (lv->nb_bricks % 20) == 6) {
						lv->bricks[y][x].with_mod = (rand() % NB_SPECIAL_MODS);
					}
					lv->nb_bricks++;
					lv->bricks[y][x].x = x* lv->bw;
					lv->bricks[y][x].y = y* lv->bh;						
				}
			}
	}
	
	app.board_height = (app.level.sy * app.level.bh) + FREE_HEIGHT + WIN_SCORE_H;
	app.board_width = app.level.sx * app.level.bw;
	
	app.winW = (app.board_width>WIN_APP_W)?app.board_width:WIN_APP_W;
	app.winH = (app.board_height>WIN_APP_H)?app.board_height:WIN_APP_H+WIN_SCORE_H;
	
	props.flags = GR_WM_FLAGS_TITLE;
	props.title = app.level.name;
	GrSetWMProperties( app.win_app, &props);
	
	GrResizeWindow( app.win_app, app.winW, app.winH );
	GrResizeWindow( app.win_score, app.winW, WIN_SCORE_H);
	GrResizeWindow( app.win_board, app.board_width, app.board_height);
	if ( app.winW > app.board_width)
		GrMoveWindow(app.win_board,app.board_x=((app.winW - app.board_width)>>1), WIN_SCORE_H+1);
	else
		GrMoveWindow(app.win_board,app.board_x=0, WIN_SCORE_H+1);
	
	/* Reconfigure la raquette avec les dims du nouveau level */
	if (app.pad_num < 2) set_pad_width_to(2);
	
	app.pady = app.board_height - (PAD_HEIGHT << 1);
	app.padx = app.board_width >> 1;
		
	if ( app.board) {
		GrDestroyWindow( app.board);
		app.board = 0;
	}
	
	free( ln);
}

/*
void msleep(long ms)
{
	struct timespec req, rem;

	req.tv_sec = ms / 1000000;
	req.tv_nsec = (ms % 1000000) * 1000000;

	while(nanosleep(&req, &rem) == -1) {
		if(errno == EINTR) {
			req.tv_sec = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
			continue;
		} else {
			perror("nanosleep() failed");
			return;
		}
	}
}
*/
#define  DELAY 10000

int step_elapsed()
{
	static struct timeval oldtime;
	
	struct timeval currenttime;
	long     a,b;

	gettimeofday( &currenttime, NULL);
	a = (currenttime.tv_sec - oldtime.tv_sec);
	b = (currenttime.tv_usec - oldtime.tv_usec);
	if ( a || b > DELAY)
	{
		oldtime = currenttime;
		return 1;
	}
	else return 0;
}	

void animate_board() {
	int i, ibr, t, t2, ox, oy, nx, ny, bx, by, bx2, by2;
	pBrick pbr;
	pBall tb, b = app.ballList;
	pBrick touch_mat[2][2]; /* la matrice des briques autour de la balle */
	pBrick br_touched[3]; /* les briques touchées (2 maxi) */
	pBonus tbnu, bonus;
	pBomb tbo, bomb;
	
	while ( b) {
		
		/* Prévision de la nouvelle position de la balle */
		nx = (int)((ox = b->x) + b->dx);
		ny = (int)((oy = b->y) + b->dy);
	 
	 	/* Vérifie les bords du tableau */
		if ( nx < 0 ) { b->dx *= -1.0f; nx = 0; }
		if ( (nx+BALL_DIAM) >= app.board_width) { b->dx *= -1.0f; nx = app.board_width - 1 - BALL_DIAM; }
		if ( ny < 0 ) { b->dy *= -1.0f; ny = 0; }
				
		/* Recherche des briques en collisions par les 4 coins de la balle */
		memset( touch_mat, 0, sizeof(touch_mat));
		
		pbr = &app.level.bricks[by=(ny / app.level.bh)][bx=(nx / app.level.bw)];
		if ( pbr->exist) touch_mat[0][0] = pbr; else touch_mat[0][0] = NULL;
		
		pbr = &app.level.bricks[by2=((ny+BALL_DIAM) / app.level.bh)][bx];
		if ( pbr->exist && (by!=by2)) touch_mat[1][0] = pbr; else touch_mat[1][0] = NULL;
		
		pbr = &app.level.bricks[by][bx2=(nx+BALL_DIAM) / app.level.bw];
		if ( pbr->exist && (bx!=bx2)) touch_mat[0][1] = pbr; else touch_mat[0][1] = NULL;
		
		pbr = &app.level.bricks[by2][bx2];
		if ( pbr->exist && ((bx!=bx2)||(by!=by2))) touch_mat[1][1] = pbr; else touch_mat[1][1] = NULL;
		
		/* On enlève la brique de téléportation dont la bille vient éventuellement */
		ibr = 0;
		if ( b->just_teleported) {
			for ( i = 0; i < 2; i++) {
				if ( touch_mat[0][i] == b->just_teleported) {
					touch_mat[0][i] = NULL; ibr=1;
				}
				if ( touch_mat[i][0] == b->just_teleported) {
					touch_mat[i][0] = NULL; ibr=1;
				}
			}
			if ( ibr==0) b->just_teleported = 0;
		}
		
		/* Gestion des rebonds */
		memset( br_touched, 0, sizeof(br_touched));
	    
		if ((touch_mat[0][0] && touch_mat[1][1]) || (touch_mat[1][0] && touch_mat[0][1])) {
/* 1 ?    or    ? 1 */
/* ? 1          1 ? */		
			b->dx *= -1.0f;
			b->dy *= -1.0f;
			if ( touch_mat[0][0]) {
				br_touched[0] = touch_mat[0][0];
				br_touched[1] = touch_mat[1][1];
			} else {
				br_touched[0] = touch_mat[0][1];
				br_touched[1] = touch_mat[1][0];				
			}
			goto apres_rebond_balle;
		}
		
		if ( (touch_mat[0][0] && touch_mat[0][1]) || (touch_mat[1][0] && touch_mat[1][1])) {
/* 1 1  or  ? ? */
/* ? ?      1 1 */
			b->dy *= -1.0f;
			br_touched[0] = touch_mat[touch_mat[0][0]==NULL][((nx+BALL_CENTER)%app.level.bw)<(app.level.bw>>1)];
			goto apres_rebond_balle;
		}

		if ( (touch_mat[0][0] && touch_mat[1][0]) || (touch_mat[0][1] && touch_mat[1][1])) {
/* 1 ?  or  ? 1 */
/* 1 ?      ? 1 */	
			b->dx *= -1.0f;
			br_touched[0] = touch_mat[((ny+BALL_CENTER)%app.level.bh)<(app.level.bh>>1)][touch_mat[0][0]==NULL];
			goto apres_rebond_balle;

		}
		
		if ( touch_mat[0][0]) {
/* 1 0 */
/* 0 0 */	if ((b->dx<0.0f)&&((nx/app.level.bw)!=(ox/app.level.bw)))
				b->dx *= -1.0f;
			if ((b->dy<0.0f)&&((ny/app.level.bh)!=(oy/app.level.bh)))
				b->dy *= -1.0f;
			br_touched[0] = touch_mat[0][0];
			goto apres_rebond_balle;
		}
		
		if ( touch_mat[1][1]) {
/* 0 0 */
/* 0 1 */	if ((b->dx>0.0f)&&(((nx+BALL_DIAM)/app.level.bw)!=((ox+BALL_DIAM)/app.level.bw)))
				b->dx *= -1.0f;
			if ((b->dy>0.0f)&&(((ny+BALL_DIAM)/app.level.bh)!=((oy+BALL_DIAM)/app.level.bh)))
				b->dy *= -1.0f;
			br_touched[0] = touch_mat[1][1];
			goto apres_rebond_balle;
		}		
			
		if ( touch_mat[0][1]) {
/* 0 1 */
/* 0 0 */	if ((b->dx>0.0f)&&(((nx+BALL_DIAM)/app.level.bw)!=((ox+BALL_DIAM)/app.level.bw)))
						b->dx *= -1.0f;
			if ((b->dy<0.0f)&&((ny/app.level.bh)!=(oy/app.level.bh)))
						b->dy *= -1.0f;
			br_touched[0] = touch_mat[0][1];
			goto apres_rebond_balle;
		}
		
		if ( touch_mat[1][0]) {
/* 0 0 */
/* 1 0 */	if ((b->dx<0.0f)&&((nx/app.level.bw)!=(ox/app.level.bw)))
						b->dx *= -1.0f;
			if ((b->dy>0.0f)&&(((ny+BALL_DIAM)/app.level.bh)!=((oy+BALL_DIAM)/app.level.bh)))
						b->dy *= -1.0f;
			br_touched[0] = touch_mat[1][0];
			goto apres_rebond_balle;
		}
			
apres_rebond_balle:

		/* action relative aux briques touchées */
		ibr = 0;
		while ( (pbr = br_touched[ibr++]) ) {
			
			switch (pbr->type) {
				case BRICK_TYPE_NONE:
						fprintf( stderr, "Error NONE (%d) brick hitten\n",t);
				case BRICK_TYPE_SOLID:
						break;
				case BRICK_TYPE_TELEPORT:
						if ( b->just_teleported) break;
						if ( app.level.nb_teleports > 1) {
							do {
								t = (rand() % app.level.nb_teleports);
							} while ( app.level.teleports[t] == pbr );
												
							b->x = app.level.teleports[t]->x + (app.level.bw >>1) - BALL_CENTER;
							b->y = app.level.teleports[t]->y + (app.level.bh >>1) - BALL_CENTER;
							b->dx += ((float)((rand()%20)-10))/10.0f;
							b->just_teleported = app.level.teleports[t];
						}
						break;
				case BRICK_TYPE_MAGIC:
						if ( ! pbr->with_mod ) 
							add_mod( rand() % NB_SPECIAL_MODS, nx, ny, b);
						break;
				default:
						pbr->type--;
					    app.score += 1;
					    app.repaint_score = 1;			
			}					    
			pbr->exist = (pbr->type!=0);
			if ( pbr->with_mod) add_mod( pbr->with_mod, nx, ny, b);

			/* on efface la brique si nécéssaire */
			if ( ! pbr->exist) {
				app.level.nb_bricks--;
				hide_brick( pbr);
			} else
				/* Repaint brick */
				GrCopyArea( app.board, app.gc, pbr->x, pbr->y, app.level.bw,
							app.level.bh, app.pixmap_bricks[pbr->type], 0, 0, 0);
		}
		
		/* Vérifie le rebond sur la raquette */
		if (((b->dy > 0) && ((ny+BALL_DIAM) > app.pady)) && 
			((nx+BALL_CENTER>=app.padx)&&(nx<=app.padx+app.pad_len))) {
			
			/* Si déja dépassé => accélèration */
			if ((oy+BALL_DIAM) <= app.pady) b->dy *= -1.0f;
			else if ((ny+BALL_DIAM) < app.pady+(PAD_HEIGHT>>1)) b->dy *= -1.5f;
			
			if ( (b->dx < 0) && (nx<app.padx+app.pad_len) &&(ox>app.padx+app.pad_len))
					b->dx *= -1.0f;
			else if ( (b->dx < 0) && (nx<app.padx+app.pad_len) &&(ox>app.padx+app.pad_len))
					b->dx *= -1.0f;
			else {
					/* rebond dépendant de la position X sur la raquette */
					t = app.padx + (t2=((app.pad_num * PAD_WIDTH_STEP + PAD_WIDTH_MIN)>>1));
					b->dx += ((float)(nx-t))/(float)(t2); 
					if ( b->dx > 5.0f) b->dx = 5.0f;
					if ( b->dx < -5.0f) b->dx = -5.0f;
			}
		}
		else /* Rebond sur le mur ? */
			if ( app.with_wall )
				if ( (b->dy > 0) && ((ny+BALL_DIAM)>=WALL_POS_Y) ) {
					ny = WALL_POS_Y - BALL_DIAM;
					b->dy *= -1.0f;
			}
				
		/* déplacement effectif de la balle */
		b->x += b->dx;
		b->y += b->dy;
		
		if ( b->y >= app.board_height ) {
			/* Si la balle est tombée, fin de partie */
			tb = b->next;
			del_ball(b);
			b = tb;
		} else {
			/* On affiche la brique teleport 
			if ( b->just_teleported )
				GrCopyArea( app.win_board, app.gc, b->just_teleported->x, b->just_teleported->y, app.level.bw,
							app.level.bh, app.pixmap_bricks[b->just_teleported->type], 0, 0, 0);
			paint_ball( b); */
			b = b->next;
		}
	}
	
	/* Animate Bonus */
	bonus= app.bonusList;
	while ( bonus) {
		//hide_bonus(bonus);
		bonus->y += bonus->speed;
		
		if ((app.with_wall && ((bonus->y+bonus->h)>=WALL_POS_Y)) || (bonus->y > app.board_height)) {
			tbnu = bonus->next;
			del_bonus(bonus);
			bonus = tbnu;
			
		} else if ( ((bonus->y+bonus->h)>app.pady) && (bonus->y<app.pady+PAD_HEIGHT) && 
			   ((bonus->x+bonus->w)>app.padx) && (bonus->x<(app.padx+app.pad_len))) {
				   
			tbnu = bonus->next;
			do_bonus(bonus);
			del_bonus(bonus);
			bonus = tbnu;
			
		} else {
			//paint_bonus(bonus);
			bonus = bonus->next;
		}
	}
	
	/* Animate Bombs */
	bomb = app.bombList;
	while ( bomb) {
		//hide_bomb(bomb);
		bomb->y -= BOMB_SPEED;
		
		pbr = NULL;
		if ( (bomb->y <= 0) || (pbr = &app.level.bricks[by=(bomb->y/app.level.bh)]
								[bx=(bomb->x/app.level.bw)])->exist) {
									
			if ( pbr && pbr->exist) {
				hide_brick( pbr);
				switch( pbr->type) {
					case BRICK_TYPE_MAGIC:
					case BRICK_TYPE_SOLID:
					case BRICK_TYPE_TELEPORT:
						break;
					default:
						app.level.nb_bricks--;
						app.repaint_score = 1;
					
				}
			}
			
			tbo = bomb->next;
			del_bomb(bomb);
			bomb = tbo;
			
		} else {
			//paint_bomb(bomb);
			bomb = bomb->next;
		}
	}
}

void do_bonus( pBonus b) {
	switch ( b->type) {
	case BONUS_TYPE_LIVE:
		app.lives++;
		app.repaint_score = 1;
		break;
	case BONUS_TYPE_MONEY:
		app.score += b->value;
		app.repaint_score = 1;
		break;
	default:
		return;
	}
	app.repaint_score=1;
}

void paint_score() {
	
	static char scorestr[100];
	GrSetGCForeground( app.gc, WHITE);
	GrFillRect( app.win_score, app.gc, 0, 0, app.winW, WIN_SCORE_H);
	GrSetGCForeground( app.gc, BLACK);
	sprintf( scorestr, "Score=%d  Lives=%d  Remains=%d", app.score, app.lives, app.level.nb_bricks);
	GrText( app.win_score, app.gc, 10, 10, scorestr, strlen(scorestr), 0);  
	app.repaint_score = 0;
}

void paint_all() {
	
	int x, y, t;
	pBall ba;
	pBomb bo;
	pBonus bu;
	
	if ( app.repaint_score) paint_score();
	
	/* On efface les bords autour du tableau */
	GrSetGCForeground (app.gc, BACK_COLOR);
	GrFillRect( app.win_app, app.gc, 0, 0, app.board_x, app.winH);
	GrFillRect( app.win_app, app.gc, 0, app.board_height + WIN_SCORE_H, app.winW, app.winH - app.board_height - WIN_SCORE_H);
	GrFillRect( app.win_app, app.gc, app.board_width+app.board_x, 0, app.winW - app.board_width - app.board_x, app.winH);
	
	if ( ! app.board ) {
		app.board = GrNewPixmap( app.board_width, app.board_height, NULL);
		
		/* On efface le tableau */
		GrSetGCForeground( app.gc, BLACK);
		GrFillRect( app.board, app.gc, 0, 0, app.board_width, app.board_height);
		
		/* Les briques */
		for( y = 0; y < MAX_BRICK_H; y++)
			for( x = 0; x < MAX_BRICK_W; x++) {
				if (app.level.bricks[y][x].exist) {
					t = app.level.bricks[y][x].type;
					GrCopyArea( app.board, app.gc, x * app.level.bw, y * app.level.bh,
								app.level.bw, app.level.bh, 
								app.pixmap_bricks[t], 0, 0, 0);
				}
		}
		
		/* Le mur */
		if ( app.with_wall ) paint_wall();
		
		/* La raquette */
		GrCopyArea( app.board, app.gc, app.padx, app.pady, 
						app.pad_len, PAD_HEIGHT, app.pixmap_rac[app.pad_num], 0, 0, 0);
	}
	
	/* Affichage de l'image du tableau */
	GrCopyArea( app.win_board, app.gc, 0, 0, app.board_width, app.board_height, app.board, 0, 0, 0);
	
	/* Ajout des balles */
	for ( ba = app.ballList; ba; ba = ba->next) paint_ball( ba);
	
	/* Ajout des bombes */
	for ( bo = app.bombList; bo; bo = bo->next) paint_bomb(bo);
	
	/* Ajout des bonus */
	for ( bu = app.bonusList; bu; bu = bu->next) paint_bonus(bu);
	
	if ( ! app.ballList ) paint_gameover();
	
	if ( app.end_of_level ) paint_endoflevel();
	
	app.repaint = 0;
}

void paint_bomb( pBomb b) {
	GrCopyArea( app.win_board, app.gc, b->x, b->y, BOMB_WIDTH, BOMB_HEIGHT, app.pixmap_bomb, 0, 0, 0);
}

void paint_bonus( pBonus b) {
	GrCopyArea( app.win_board, app.gc, b->x, b->y, b->w, b->h, app.pixmap_bonus[b->type], 0, 0, 0);
}

void paint_ball( pBall b) {
	GrCopyArea( app.win_board, app.gc, b->x, b->y, BALL_DIAM, BALL_DIAM, app.pixmap_ball, 0, 0, 0);
}

void paint_wall() {
	int x;
	for( x = 0; x < app.board_width; x+=WALL_WIDTH) {
		GrCopyArea( app.board, app.gc, x, WALL_POS_Y, app.board_width, WALL_HEIGHT, app.pixmap_wall, 0, 0, 0);
	}
}

void paint_gameover() {

	GrSetGCForeground(app.gc, WHITE);
	GrFillRect( app.win_board, app.gc, GAMEOVER_X, GAMEOVER_Y, GAMEOVER_W, GAMEOVER_H); 
	GrSetGCForeground(app.gc, BLACK);
	GrRect( app.win_board, app.gc, GAMEOVER_X + 2 , GAMEOVER_Y + 2, GAMEOVER_W - 4, GAMEOVER_H - 4); 
	GrText( app.win_board, app.gc, GAMEOVER_X+GAMEOVER_M, GAMEOVER_Y+GAMEOVER_M, "Game Over", 9, 0);
}

void paint_endoflevel() {

	char bonus_str[20];
	
	GrSetGCForeground(app.gc, WHITE);
	GrFillRect( app.win_board, app.gc, GOODWIN_X, GOODWIN_Y, GOODWIN_W, GOODWIN_H); 
	GrSetGCForeground(app.gc, BLACK);
	GrRect( app.win_board, app.gc, GOODWIN_X + 2 , GOODWIN_Y + 2, GOODWIN_W - 4, GOODWIN_H - 4); 
	GrText( app.win_board, app.gc, GOODWIN_X+GOODWIN_M, GOODWIN_Y+GOODWIN_M, "Good !", 6, 0);
	sprintf( bonus_str, "Bonus %d", app.level.bonus_level);
	GrText( app.win_board, app.gc, GOODWIN_X+GOODWIN_M,
								    GOODWIN_Y+GOODWIN_M+15, bonus_str, strlen(bonus_str), 0);
}

/*
void hide_ball( pBall b) {
	GrFillRect( app.board, app.gc, b->x, b->y, BALL_DIAM, BALL_DIAM);
}

void hide_bonus( pBonus b) {
	GrFillRect( app.win_board, app.gc, b->x, b->y, b->w, b->h);
}

void hide_bomb( pBomb b) {
	GrFillRect( app.win_board, app.gc, b->x, b->y, BOMB_WIDTH, BOMB_HEIGHT);
}

void hide_pad(int x) {
	GrFillRect(app.win_board, app.gc, app.padx, app.pady, app.pad_len, PAD_HEIGHT);		
}
*/

void hide_pad() {
	GrSetGCForeground( app.gc, BLACK);
	GrFillRect(app.board, app.gc, app.padx, app.pady, app.pad_len, PAD_HEIGHT);		
}
void hide_wall() {
	GrSetGCForeground( app.gc, BLACK);
	GrFillRect( app.board, app.gc, 0, app.board_height - WALL_HEIGHT, app.board_width, WALL_HEIGHT);
}

void hide_brick( pBrick b) {
	GrSetGCForeground( app.gc, BLACK);
	b->exist = 0;
	GrFillRect( app.board, app.gc, 
				b->x, b->y, app.level.bw, app.level.bh);
}

void add_mod( int type, int x, int y, pBall b) {
	pMod m;
	int duration;
	
	if ((type <= SPEC_MOD_NONE) || (type >= NB_SPECIAL_MODS)) return;
	
	duration = MOD_MIN_TIME + MOD_ADD_TIME * (rand()%100);
	m = app.activatedModeList;
	while ( m) {
		if ( m->type == type) {
			m->remaining_time +=duration;
			return;
		}
		m = m->next;
	}
	
	/* Mods with out duration */
	switch( type) {
		case SPEC_MOD_EXTRA_BALL:
			add_ball(x, y, 1.0f - ((float)(rand() % 200))/100.0f, b->dy);
			return;
		case SPEC_MOD_NEXT_LEVEL:
			app.level.nb_bricks = 0;
			return;
		case SPEC_MOD_SHRINK: 
			set_pad_width_to(app.pad_len-1);
			return;
		case SPEC_MOD_GROW:
			set_pad_width_to(app.pad_len+1);
			return;
		case SPEC_MOD_EXTRA_LIVE:
			add_bonus( x, y, BONUS_TYPE_LIVE, 1);
			return;
		case SPEC_MOD_BONUS:
			add_bonus( x, y, BONUS_TYPE_MONEY, 1 + (rand() % 20));
			return;
	}
	
	/* Mod with duration */
	m = my_malloc(sizeof(sMod));
	m->type = type;
	m->remaining_time = duration;
	m->next = app.activatedModeList; 
	app.activatedModeList = m;
	
	switch ( type) {
		case SPEC_MOD_QUICK:
			b = app.ballList;
			while ( b) {
				b->dx *= 1.3f;
				b->dy *= 1.3f;
				b=b->next;
			}
			break;
		case SPEC_MOD_SLOW:
			b = app.ballList;
			while ( b) {
				b->dx /= 1.3f;
				b->dy /= 1.3f;
				b=b->next;
			}
			break;
		case SPEC_MOD_TINY: 
			set_pad_width_to(PAD_MIN_STEP);
			break;
		case SPEC_MOD_LARGE:
			set_pad_width_to(PAD_MAX_STEP);
			break;
		case SPEC_MOD_WALL:
			app.with_wall = 1;
			paint_wall();
			break;
		case SPEC_MOD_BOMB:
			m->data = 1;
			break;
		default:
			fprintf(stderr, "Not implemented MOD %d\n", type);
	}
}

void set_pad_width_to(int val) {
	if ( val < 0) val = 0;
	if ( val > (PAD_MAX_STEP-1)) val = PAD_MAX_STEP-1;
	
	//hide_pad();
	app.pad_num = val;
	app.pad_len = app.pad_num * PAD_WIDTH_STEP + PAD_WIDTH_MIN ;
	paint_pad();
}

void update_mods() {
	pMod m1, m = app.activatedModeList;

	while ( m) {
		if ( m->remaining_time-- <= 1) {
			m1 = m->next;
			del_mod(m, 1);
			m = m1;
		} else {
			switch ( m->type) {
			case SPEC_MOD_BOMB:
				if ( m->data-- <= 1) {
					add_bomb();
					m->data = BOMB_FREQ;
				}
			}
			m = m->next;
		}
	}
}

void del_mod( pMod m, int with_clean_action) {

	pBall b;
	pMod m2;
	
	if ( with_clean_action)
		switch ( m->type) {
			case SPEC_MOD_QUICK:
				b = app.ballList;
				while ( b) {
					b->dx /= 1.3f;
					b->dy /= 1.3f;
					b=b->next;
				}
				break;
			case SPEC_MOD_SLOW:
				b = app.ballList;
				while ( b) {
					b->dx *= 1.3f;
					b->dy *= 1.3f;
					b=b->next;
				}
				break;
			case SPEC_MOD_TINY:
			case SPEC_MOD_LARGE:
				set_pad_width_to(2);
				break;
			case SPEC_MOD_WALL:
				hide_wall();
				app.with_wall = 0;
				break;	
		}
		
	/* destruction de m de la liste */
	if ( m == app.activatedModeList ) app.activatedModeList = m->next;
	else {
			m2 = app.activatedModeList;
			while ( m != m2->next) m2 = m2->next;
			m2->next = m->next;
	}	
	free(m);
}

void add_ball( int x, int y, float dx, float dy) {
	if ( app.nb_balls < MAX_BALLS) {
		pBall b = my_malloc(sizeof(sBall));
		b->x = x;
		b->y = y;	
		b->dx = dx;
		b->dy = dy;
		b->next = app.ballList;
		app.ballList = b;
		app.nb_balls++;
	}
}

void add_bomb() {
	pBomb b = my_malloc(sizeof(sBomb));
	b->x = app.padx + (app.pad_len>>1);
	b->y = app.pady - BOMB_HEIGHT - 2;
	b->next = app.bombList;
	paint_bomb( app.bombList = b);
}

void add_bonus( int x, int y, int type, int value) {
	pBonus b = my_malloc(sizeof(sBonus));
	b->type = type;
	b->x = x;
	b->y = y;
	b->value = value;
	b->speed = 0.5f + ((float)(rand() % 200))/100.0f;
	
	switch (type){
	default: fprintf(stderr,"add_bonus: unknow bonus type (%d) ", type);
	case 0: b->w = b->h = BONUS_1_SIZE; break;
	case 1: b->w = BONUS_2_W; b->h = BONUS_2_H; break;	
	}

	b->next = app.bonusList;
	app.bonusList = b;
}

void del_ball( pBall b) {
	
	pBall b2 = app.ballList;
	
	if (b == b2) app.ballList = b->next;
	else {
		while ( b2->next != b) b2 = b2->next;
		b2->next = b->next;
	}
	free(b);
	app.nb_balls--;
}

void del_bomb( pBomb b) {

	pBomb b2 = app.bombList;
	
	if (b == b2) app.bombList = b->next;
	else {
		while ( b2->next != b) b2 = b2->next;
		b2->next = b->next;
	}
	free(b);
}	

void del_bonus( pBonus b) {
	
	pBonus b2 = app.bonusList;
	
	if ( b2 == b ) app.bonusList = b->next;
	else {
		while ( b2->next != b) b2 = b2->next;
		b2->next = b->next;
	}
	free(b);
}	

void event_handler (GR_EVENT *event)
{	
	switch (event->type) {
		case GR_EVENT_TYPE_UPDATE:
			if ( ((GR_EVENT_UPDATE *)event)->utype == GR_UPDATE_SIZE ) {
				app.winW = ((GR_EVENT_UPDATE *)event)->width;
				app.winH = ((GR_EVENT_UPDATE *)event)->height;
				app.board_x = (app.winW - app.board_width)>>1;
				if ( app.board_x < 0 ) app.board_x = 0;
				GrMoveWindow( app.win_board, app.board_x, WIN_SCORE_H + 1);
				GrResizeWindow( app.win_score, app.winW, WIN_SCORE_H);
			} else break;
		case GR_EVENT_TYPE_EXPOSURE:
			app.repaint = 1;
			break;
		case GR_EVENT_TYPE_CLOSE_REQ:
			GrClose();
			exit (0);
		case GR_EVENT_TYPE_BUTTON_UP:
			//app.paused = 1;
		    if ( event->mouse.y <= WIN_SCORE_H) {
				app.paused = 1;
				
			}break;
		case GR_EVENT_TYPE_BUTTON_DOWN:		
			if ( app.end_of_level) {
				load_level( app.level.next);
				add_ball( app.board_width >> 1, app.pady - PAD_HEIGHT, -1.0f, -1.0f);
				app.end_of_level = 0;
			}
			else if ( app.paused == 1) app.paused = 0;
			
		case GR_EVENT_TYPE_MOUSE_MOTION:
		case GR_EVENT_TYPE_MOUSE_POSITION:
			event->mouse.x -= app.board_x + (app.pad_len>>1);
			
			if (event->mouse.x < 0) event->mouse.x = 0;
			if (event->mouse.x > (app.board_width - app.pad_len))
						event->mouse.x = app.board_width - app.pad_len;
				
			if ( app.padx != event->mouse.x) {
				hide_pad();
				app.padx = event->mouse.x;
				paint_pad();
				app.repaint = 1;
			}
			break;
		default:
			fprintf(stderr, "Got unknown event %d\n", event->type);
			break;
	}
}

void paint_pad() {
	GrCopyArea( app.board, app.gc, app.padx, app.pady, 
				app.pad_len, PAD_HEIGHT, app.pixmap_rac[app.pad_num], 0, 0, 0);
}

int load_sprites( char *filename) {

	int x, y, n = 0;
	
	GR_DRAW_ID images;
	/* if (access(filename, F_OK) != 0) {
		printf("PAS CVOOL %s\n", filename);	
	}
	
	// images = GrLoadImageFromFile(filename, 0);
	
	images = GrNewPixmap(100, 100, NULL);
	GrDrawImageFromFile(images, app.gc, 0, 0, 100, 100, filename, 0);
	
	if ( ! images) {
		perror("Could not load sprites image...\n");
		return 0;
	}
	*/
	images = GrNewPixmap(image_nanobrick_images.width, image_nanobrick_images.height, NULL);
	GrDrawImageBits(images, app.gc, 0, 0, &image_nanobrick_images);
		
	
	/* Make bricks images */
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

int main (int argc, char**argv)
{
	GR_EVENT ev;
	int opt;
	char *level = NULL, *sprites = NULL;
	
	if (GrOpen() < 0) {
		fprintf (stderr, "GrOpen failed");
		exit(1);
	}
	
	app.gc = GrNewGC();
	GrSetGCUseBackground (app.gc, GR_FALSE);
		
	app.win_app = GrNewWindowEx ( 
	       GR_WM_PROPS_APPFRAME | GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX
			 | GR_WM_PROPS_NOBACKGROUND, "Nanobrick Window",
						 GR_ROOT_WINDOW_ID, 20, 20, WIN_APP_W, WIN_APP_H, WHITE);

	GrSelectEvents (app.win_app, GR_EVENT_MASK_BUTTON_DOWN | 
							 GR_EVENT_MASK_BUTTON_UP |
							 GR_EVENT_MASK_MOUSE_MOTION |
							 GR_EVENT_MASK_MOUSE_POSITION |
							 GR_EVENT_MASK_EXPOSURE |
							 GR_EVENT_MASK_UPDATE |
							 GR_EVENT_MASK_CLOSE_REQ);

	app.win_score = GrNewWindow( app.win_app, 0, 0, WIN_APP_W, WIN_SCORE_H, 1, WHITE, BLACK);
	app.win_board = GrNewWindow( app.win_app, 0, 12, WIN_APP_W, WIN_APP_H, 1, WHITE, BLACK);

	while ((opt = getopt(argc, argv, "l:s:")) != -1) {
	   switch (opt) {
	   case 'l':
		   level = optarg;
		   break;
	   case 's':
		   sprites = optarg;
		   printf("[%s]\n", optarg);
		   break;
	   default: /* '?' */
		   fprintf(stderr, "Usage: %s [-s sprite_filename] [-l game_name]\n", argv[0]);
		   exit(EXIT_FAILURE);
	   }
	}

    srand( time(NULL));
    load_level( level);
    if ( sprites ) load_sprites( sprites);
	else load_sprites( "nanobrick/nanobrick_images.gif"); 

	app.lives = 3;
	add_ball( app.board_width >> 1, app.pady - PAD_HEIGHT, -1.0f, -1.0f);
	app.repaint_score = 1;

	GrMapWindow (app.win_app);
	GrMapWindow (app.win_score);
	GrMapWindow (app.win_board);

	do {
		GrGetNextEventTimeout(&ev, 25);
		if ( ev.type != GR_EVENT_TYPE_TIMEOUT) event_handler(&ev);
		
		if ( step_elapsed() && (app.ballList) ) {	
			if ( ! app.paused ) { 
			
				update_mods();
				animate_board();			
				if ( app.ballList == NULL) {
					app.paused = 1;
					if ( --app.lives ) {
						app.repaint_score = 1; 
						add_ball( app.board_width >> 1, app.pady - PAD_HEIGHT, -1.0f, -1.0f);
					}
				}
				if ( app.level.nb_bricks <= 0) {
					app.end_of_level=1;
					app.paused = 1;
					app.score += app.level.bonus_level;
					//while ( app.ballList) del_ball( app.ballList);
				}
				app.repaint = 1;
			}
			if ( app.repaint ) paint_all();
		}
	} while (1);
	
	return 0;
}
