/*
 * OpenTom nanobrick
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

#ifndef __SAMPLEX_H__
#define __SAMPLEX_H__

#define MWINCLUDECOLORS
#include <microwin/nano-X.h>

#define PAD_HEIGHT 10
#define PAD_WIDTH_MIN 15
#define PAD_WIDTH_STEP 5
#define PAD_MAX_STEP 5
#define PAD_MIN_STEP 0

#define FREE_HEIGHT (PAD_HEIGHT << 2)

#define DEF_BRICK_H 10
#define DEF_BRICK_W 20

#define MAX_BRICK_W 15
#define MAX_BRICK_H 19

#define MAX_TELEPORT 10

#define MAX_BALLS	20

#define BALL_CENTER 2
#define BALL_DIAM	5

#define BOMB_WIDTH  6
#define BOMB_HEIGHT 10
#define BOMB_SPEED  3
#define BOMB_FREQ	25

#define WALL_WIDTH  9
#define WALL_HEIGHT 6
#define WALL_POS_Y	(app.board_height - WALL_HEIGHT - 2)

#define BRICK_TYPE_NONE		0
#define BRICK_TYPE_NORMAL   1
#define BRICK_TYPE_2X       2
#define BRICK_TYPE_3X	    3
#define BRICK_TYPE_4X       4
#define BRICK_TYPE_5X	    5
#define BRICK_TYPE_6X       6
#define BRICK_TYPE_SOLID    7
#define BRICK_TYPE_TELEPORT 8
#define BRICK_TYPE_MAGIC	9
#define NB_BRICK_TYPES		10

#define WIN_SCORE_H	12
#define WIN_APP_H	200
#define WIN_APP_W	200
#define BACK_COLOR	GR_RGB(200,200,200)

#define SPEC_MOD_NONE		0
#define SPEC_MOD_BONUS		1
#define SPEC_MOD_QUICK		2
#define SPEC_MOD_SLOW		3
#define SPEC_MOD_TINY		4
#define SPEC_MOD_LARGE		5
#define SPEC_MOD_EXTRA_BALL 6
#define SPEC_MOD_NEXT_LEVEL 7
#define SPEC_MOD_WALL 		8
#define SPEC_MOD_BOMB 		9
#define SPEC_MOD_GROW		10
#define SPEC_MOD_SHRINK		11
#define SPEC_MOD_EXTRA_LIVE 12
#define NB_SPECIAL_MODS		13

#define MOD_MIN_TIME	1000
#define MOD_ADD_TIME	100

#define BONUS_1_SIZE  5
#define BONUS_2_W	  9
#define BONUS_2_H	  4
#define MAX_BONUS_SPEED   3

#define MAX_BONUS_TYPE		2
#define BONUS_TYPE_MONEY	0
#define BONUS_TYPE_LIVE		1

#define GAMEOVER_M  30
#define GAMEOVER_W  110
#define GAMEOVER_H  50
#define GAMEOVER_X	((app.board_width-GAMEOVER_W)>>1)
#define GAMEOVER_Y  ((app.board_height-GAMEOVER_H)>>1)

#define GOODWIN_M	30
#define GOODWIN_W	110
#define GOODWIN_H	65
#define GOODWIN_X	((app.board_width-GOODWIN_W)>>1)
#define GOODWIN_Y	((app.board_height-GOODWIN_H)>>1)

typedef struct {
	int x, y;
	int with_mod;
	int type;
	int exist;
} sBrick, *pBrick;

typedef struct _sBomb {
	int x, y;
	struct _sBomb *next;
} sBomb, *pBomb;

typedef struct _sBonus {
	int type, w, h, value;
	float x, y, speed;
	struct _sBonus *next;
} sBonus, *pBonus;

typedef struct _sBall {
	pBrick just_teleported;
	float x, y, dx, dy;
	struct _sBall *next;
} sBall, *pBall, **ppBall;

typedef struct _sMod {
	int type, remaining_time, data;
	struct _sMod *next;
} sMod, *pMod;

typedef struct {
	int bw, bh; /* taille des briques */
	int sx, sy; /* dimension du tableau de briques */
	sBrick bricks[MAX_BRICK_H][MAX_BRICK_W];
	pBrick teleports[MAX_TELEPORT];
	int nb_bricks, nb_teleports; 
	char *name;
	int bonus_level;
	char *next;
} sLevel, *pLevel;
	
typedef struct {
	GR_WINDOW_ID win_app, win_board, win_score;
	GR_GC_ID gc;
	GR_COORD board_x, board_width, board_height;
	GR_COORD winH, winW; /* height and width of main window */
	GR_DRAW_ID pixmap_bricks[10]; /* les sprites des briques */
	GR_DRAW_ID pixmap_rac[5]; /* les sprites de la raquette */
	GR_DRAW_ID board, pixmap_ball, pixmap_bomb, pixmap_bonus[2], pixmap_wall;
	pBomb bombList;
	pBonus bonusList;
	pMod activatedModeList;
	pBall ballList;
	int padx, pady;
	int pad_num, pad_len;
	int end_of_level, paused, with_wall;
	sLevel level;
	int board_ok, repaint, repaint_score, score, lives, nb_balls;
} sAppState, *pAppState;

void event_handler (GR_EVENT *event);
int load_sprites( ); // char *filename);
void load_level();
void add_ball( int x, int y, float dx, float dy);
void add_bomb();
void add_bonus( int x, int y, int type, int value);
void add_mod( int type, int x, int y, pBall b);
void del_bomb( pBomb b);
void del_bonus( pBonus b);
void del_ball( pBall b);
void del_mod( pMod m, int with_clean_action);
void paint_all();
void paint_score();
void paint_wall();
void paint_pad();
void paint_bomb( pBomb b);
void paint_ball( pBall b);
void paint_bonus( pBonus b);
void paint_gameover();
void paint_endoflevel();
void hide_pad();
void show_pad();
void hide_wall();
void hide_brick( pBrick b);
void do_bonus( pBonus b);
void do_mods();
void set_pad_width_to(int val);

#endif
