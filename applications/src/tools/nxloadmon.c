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



/* The obligatory TODO list:
   - Allow the user to update speed via command line 
   - Put in handlers for other Unix like kernels that may not have
   a similar /proc/stat file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define MWINCLUDECOLORS
#include <microwin/nano-X.h>
//#include <wm/nxlib.h>

#define DEF_TITLE 	"CPU"
#define DEF_STYLE	(GR_WM_PROPS_NOAUTOMOVE | \
			GR_WM_PROPS_NOFOCUS | \
			GR_WM_PROPS_BORDER | \
			GR_WM_PROPS_CAPTION | \
			GR_WM_PROPS_CLOSEBOX)
#define DEF_COLOR	GR_COLOR_WINDOW
#define DEF_GEOMETRY	"77x60-0-0"
//#define DEF_GEOMETRY  "100x50-0-0"

#define BACKLOG 	78
#define CTIMEOUT 	500
#define BGCOLOR         MWRGB(0xFF,0xFF,0xFF)

/* application initialization data
static nxARGS args[] = {
    nxTITLE(DEF_TITLE),
    nxGEOMETRY(DEF_GEOMETRY),
    nxBACKGROUND(DEF_COLOR),
    nxSTYLE(DEF_STYLE),
    nxEND
}; */

double queue[BACKLOG];
int qhead = 1;
int qtail = 0;

/* Used to hold the delta data from /proc/stat */

struct
{
    unsigned long user;
    unsigned long nice;
    unsigned long sys;
    unsigned long idle;
    unsigned char valid;
}
cpudata;

/* File descriptor */

int loadavg;

static GR_WINDOW_ID mywin;
static GR_GC_ID gc;
static GR_WINDOW_INFO info;
#define XSIZE	info.width
#define YSIZE	info.height

static void expose_points(void);
static void insert_point(double value);

static double
get_load()
{
    unsigned long user, nice, sys, idle;
    double total = 0, busy = 0;

    char str[BUFSIZ];
    char *c;

    /* Very tricky.  We read the first line from /proc/stat */
    /* and parse it up */

    lseek((int) loadavg, 0, SEEK_SET);
    read((int) loadavg, str, BUFSIZ - 1);

    /* Now skip over "cpu" */
    for (c = str; *c != ' '; c++)
	continue;
    c++;

    /* Get the new values */

    user = strtoul(c, &c, 0);
    nice = strtoul(c, &c, 0);
    sys = strtoul(c, &c, 0);
    idle = strtoul(c, &c, 0);

    /* Get the delta with the old values */
    if (cpudata.valid) {
	unsigned long duser, dnice, dsys, didle;

	duser = abs(user - cpudata.user);
	dnice = abs(nice - cpudata.nice);
	dsys = abs(sys - cpudata.sys);
	didle = abs(idle - cpudata.idle);

	busy = (double) duser + dnice + dsys;
	total = (double) busy + didle;
    } else
	total = 0;

    /* And fill up the struct with the new values */
    cpudata.user = user;
    cpudata.nice = nice;
    cpudata.sys = sys;
    cpudata.idle = idle;
    cpudata.valid = 1;

    if (total == 0)
	return (0);

    /* Return the % of cpu use */
    return (busy / total);
}

int
main(int ac, char **av)
{
    GR_EVENT event;

    /* Now open up a handle to our friend, /proc/loadavg */
    loadavg = open("/proc/stat", O_RDONLY);

    if (loadavg == -1) {
	perror("couldn't open /proc/stat");
	exit(1);
    }

    /* Open up the graphics */
    if (GrOpen() < 0) {
	fprintf(stderr, "cannot open graphics\n");
	exit(1);
    }

    /* read arglist and create application window */
    mywin = GrNewWindow(GR_ROOT_WINDOW_ID, 50, 50, 78, 50, 1, BLACK, WHITE);
	//nxCreateAppWindow(&ac, &av, args);

    GrSelectEvents(mywin, GR_EVENT_MASK_TIMEOUT | GR_EVENT_MASK_EXPOSURE |
		   GR_EVENT_MASK_CLOSE_REQ);
    GrMapWindow(mywin);

    /* Get a graphics context */
    gc = GrNewGC();

    GrGetWindowInfo(mywin, &info);
    memset(&cpudata, sizeof(cpudata), 0);
    get_load();

    while (1) {
	GrGetNextEventTimeout(&event, CTIMEOUT);

	switch (event.type) {
	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);

	case GR_EVENT_TYPE_EXPOSURE:
	    expose_points();
	    break;

	case GR_EVENT_TYPE_TIMEOUT:
	    insert_point(get_load());

#ifdef NOTUSED
	    load_array[load_array_head] = get_load();
	    if (++load_array_head == BACKLOG)
		load_array_head = 0;
	    if (++load_array_tail == BACKLOG)
		load_array_tail = 0;
#endif

	    expose_points();
	    break;

	}

    }
}

/* For the example,
   g goes from 221 -- 237
   b goes from 0 to 237
*/

static void
expose_points(void)
{
    int h, x = 0;
    int r = 0x00;
    int g = 0x66;
    int b = 0xCC;

    int rdiff = (204) / 38;
    int gdiff = (204 - 102) / 38;

    int point = qtail;

    while (1) {
	h = (queue[point] * YSIZE);

	GrSetGCForeground(gc, MWRGB(r, g, b));
	GrLine(mywin, gc, x, YSIZE - h, x, YSIZE);

	GrSetGCForeground(gc, BGCOLOR);
	GrLine(mywin, gc, x, 0, x, YSIZE - h - 1);

	if (point == qhead)
	    break;

	if (--point < 0)
	    point = BACKLOG - 1;

	x++;

#ifdef NOTUSED
	if (x < 38) {
	    g -= gdiff;
	    r -= rdiff;
	} else {
	    g += gdiff;
	    r += rdiff;
	}
#endif
    }
}

static void
insert_point(double value)
{
    if (--qhead < 0)
	qhead = (BACKLOG - 1);

    if (--qtail < 0)
	qtail = (BACKLOG - 1);

    queue[qhead] = value;
}
