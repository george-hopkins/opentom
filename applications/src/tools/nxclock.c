/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */



#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "vertex.h"
#include "nxclock.h"
#define MWINCLULDECOLORS
#include <microwin/nano-X.h>
#include <wm/nxlib.h>

#define DEF_STYLE	(GR_WM_PROPS_APPWINDOW |\
			 GR_WM_PROPS_NOAUTORESIZE |\
			 GR_WM_PROPS_NOAUTOMOVE)
#define DEF_GEOMETRY	"100x100-0+0"
#define DEF_TITLE	"Clock"

/* application initialization data
static nxARGS args[] = {
    nxTITLE(DEF_TITLE),
    nxGEOMETRY(DEF_GEOMETRY),
    nxBACKGROUND(GR_COLOR_WINDOW),
    nxSTYLE(DEF_STYLE),
    nxEND
}; */

GR_SIZE width;
GR_SIZE height;

static struct clock_time newtime = { 0, 0, 0 };

const float hourhand[4][2] = { {-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -7.0f} };
const float minhand[4][2] = { {-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -11.5f} };
const float sechand[4][2] = { {-0.1f, 0}, {0, 2.0f}, {0.1f, 0}, {0, -11.5f} };

static void
drawhand(double ang, const float v[][2], GR_COLOR fill,
	 GR_COLOR line, GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    int i;

    push_matrix();
    rotate(ang);

    GrSetGCForeground(gc, fill);
    begin_polygon();
    for (i = 0; i < 4; i++)
	vertex(v[i][0], v[i][1]);
    end_polygon(pmap, gc);
    GrSetGCForeground(gc, line);
    begin_loop();
    for (i = 0; i < 4; i++)
	vertex(v[i][0], v[i][1]);
    end_loop(pmap, gc);
    pop_matrix();
}

static uchar
type()
{
    return ROUND_CLOCK;
}

void
draw_clock_hands(GR_COLOR fill, GR_COLOR line, GR_WINDOW_ID pmap, GR_GC_ID gc)
{

    drawhand(-360 * (newtime.hour_ + newtime.minute_ / 60.0) / 12, hourhand,
	     fill, line, pmap, gc);
    drawhand(-360 * (newtime.minute_ + newtime.second_ / 60.0) / 60, minhand,
	     fill, line, pmap, gc);
    drawhand(-360 * (newtime.second_ / 60.0), sechand, fill, line, pmap, gc);
}


static void
rect(double x, double y, double w, double h, GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    double r = x + w;
    double t = y + h;
    begin_polygon();
    vertex(x, y);
    vertex(r, y);
    vertex(r, t);
    vertex(x, t);
    end_polygon(pmap, gc);
}

void
reset_values(int h, int m, int s)
{
    if (h != newtime.hour_ || m != newtime.minute_ || s != newtime.second_) {
	newtime.hour_ = h;
	newtime.minute_ = m;
	newtime.second_ = s;
    }
}

static void
tick()
{
    struct timeval t;
    struct tm *timeofday;

    gettimeofday(&t, 0);
    timeofday = localtime((const time_t *) &t.tv_sec);
    reset_values(timeofday->tm_hour, timeofday->tm_min, timeofday->tm_sec);
}

void
draw_clock(int x, int y, int w, int h, GR_WINDOW_ID pmap, GR_GC_ID gc,
	   GR_WINDOW_ID window)
{
    int i;

    GrSetGCForeground(gc, GrGetSysColor(GR_COLOR_WINDOW));
    GrFillRect(pmap, gc, 0, 0, w, h);

    tick();
    push_matrix();
    translate(x + w / 2.0 - .5, y + h / 2.0 - .5);
    scale_xy((w - 1) / 28.0, (h - 1) / 28.0);
    if (type() == ROUND_CLOCK) {
	GrSetGCForeground(gc, BLACK);
	begin_polygon();
	circle(0, 0, 14, pmap, gc, w, h);
	end_polygon(pmap, gc);
	GrSetGCForeground(gc, BLACK);
	begin_loop();
	circle(0, 0, 14, pmap, gc, w, h);
	end_loop(pmap, gc);
    }
    //draw the shadows
    push_matrix();
    translate(0.60, 0.60);
    draw_clock_hands(LTGRAY, LTGRAY, pmap, gc);
    pop_matrix();
    //draw the tick marks
    push_matrix();
    GrSetGCForeground(gc, BLACK);
    for (i = 0; i < 12; i++) {
	if (6 == i)
	    rect(-0.5, 9, 1, 2, pmap, gc);
	else if (3 == i || 0 == i || 9 == i)
	    rect(-0.5, 9.5, 1, 1, pmap, gc);
	else
	    rect(-0.25, 9.5, .5, 1, pmap, gc);
	rotate(-30);
    }
    pop_matrix();
    //draw the hands
    draw_clock_hands(GRAY, BLACK, pmap, gc);
    pop_matrix();
    GrCopyArea(window, gc, 0, 0, w, h, pmap, 0, 0, MWROP_SRCCOPY);
}

GR_WINDOW_ID
resize(GR_SIZE w, GR_SIZE h, GR_WINDOW_ID pmap)
{
    width = w;
    height = h;
    if (pmap)
	GrDestroyWindow(pmap);
    return GrNewPixmap(w, h, NULL);
}

int
main(int ac, char **av)
{
    GR_EVENT event;
    GR_GC_ID gc;
    GR_WINDOW_ID pmap;
    GR_WINDOW_ID window;
    GR_WINDOW_INFO info;

    if (GrOpen() < 0) {
	fprintf(stderr, "cannot open graphics\n");
	exit(1);
    }

    window = nxCreateAppWindow(&ac, &av, args);
    gc = GrNewGC();
    GrSetGCForeground(gc, GrGetSysColor(GR_COLOR_WINDOW));
    GrSetGCBackground(gc, GrGetSysColor(GR_COLOR_WINDOWTEXT));

    GrSelectEvents(window, GR_EVENT_MASK_EXPOSURE |
		   GR_EVENT_MASK_UPDATE | GR_EVENT_MASK_CLOSE_REQ);

    GrGetWindowInfo(window, &info);
    pmap = resize(info.width, info.height, 0);

    GrMapWindow(window);

    while (1) {
	GrGetNextEventTimeout(&event, 500L);

	switch (event.type) {
	case GR_EVENT_TYPE_EXPOSURE:
	case GR_EVENT_TYPE_TIMEOUT:
	    draw_clock(0, 0, width, height, pmap, gc, window);
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);

	case GR_EVENT_TYPE_UPDATE:
	    switch (event.update.utype) {
	    case GR_UPDATE_SIZE:
		pmap = resize(event.update.width, event.update.height, pmap);
		break;
	    }
	}
    }
    return 0;
}
