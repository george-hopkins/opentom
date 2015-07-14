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



/*
**
** Imported "Include" Files
**
*/
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "cli.h"
#include "filetools.h"
#include "keymap.h"
#define MWINCLUDECOLORS
#include <microwin/nano-X.h>
#include "nxlib.h"



/*
**
** External Declarations
**
*/
extern int KbdOpen(void);
extern int KbdWrite(int keycode, int pushed);


/*
**
** Local Constant Definitions
**
*/
#define FLASH_DELAY 100000
#define MAX_KEYMAPS 32



/*
**
** Local Enumeration Definitions
**
*/



/*
**
** Local Structure Definitions
**
*/



/*
**
** Local Variable Declarations
**
*/
static nxARGS nxargs[] = {
    {nxtypeTITLE | nxSTR, "-title", "Popup Keyboard"},
    {nxtypeGEOMETRY | nxINT, "-geom", "32x32+0-0"},
    {nxtypeBACKGROUND | nxINT, "-background", (char *) GR_COLOR_WINDOW},
    {nxtypeSTYLE | nxINT, "-style", (char *) (GR_WM_PROPS_NOFOCUS |
					      GR_WM_PROPS_NOAUTOMOVE |
					      GR_WM_PROPS_APPFRAME)},
    {0, NULL, NULL}
};
static GR_GC_ID gc;
static GR_WINDOW_ID w;
static KeymapHandle *keymaps[MAX_KEYMAPS];
static GR_IMAGE_ID images[MAX_KEYMAPS];
static GR_PIXELVAL *pixels;
static int current_map;
static int old_map;
static int extra_width, extra_height;

static char cpGeom[32 + 1];


/*
**
** This function will invert the image of the specified area.
**
*/
void invertArea(int ulx, int uly, int lrx, int lry)
{
    int count, height, size, width;

    // read the affected area
    width = (lrx - ulx) + 1;
    height = (lry - uly) + 1;
    GrReadArea(w, ulx, uly, width, height, pixels);

    // invert the pixels
    size = width * height -1;
    do pixels[size--] ^= 0xffff; while ( size >= 0);

    GrArea(w, gc, ulx, uly, width, height, pixels, MWPIXEL_FORMAT);
}



/*
**
** This function will invert the image of the specified key.
**
*/
static int
invertKey(int keynum)
{
    KeymapEntry *kentry;

    kentry = keymaps[current_map]->keys + keynum;
    invertArea(kentry->ulx, kentry->uly, kentry->lrx, kentry->lry);
    return (0);
}



/*
**
**
**
*/
static int
updateWindow(void)
{
    GR_WINDOW_INFO pinfo, winfo;

    GrGetWindowInfo(w, &winfo);
    if (winfo.parent != GR_ROOT_WINDOW_ID) {
	if (extra_width == -1) {
	    GrGetWindowInfo(winfo.parent, &pinfo);
	    extra_width = pinfo.width - winfo.width;
	    extra_height = pinfo.height - winfo.height;
	}

	if ((winfo.width != keymaps[current_map]->width) ||
	    (winfo.height != keymaps[current_map]->height)) {
	    GrResizeWindow(winfo.parent,
			   keymaps[current_map]->width + extra_width,
			   keymaps[current_map]->height + extra_height);
	    GrFlush();
	}
    }

    GrDrawImageToFit(w, gc, 0, 0, -1, -1, images[current_map]);
    if (keymaps[current_map]->highlight1 != -1)
	invertKey(keymaps[current_map]->highlight1);
    if (keymaps[current_map]->highlight2 != -1)
	invertKey(keymaps[current_map]->highlight2);
    if (keymaps[current_map]->highlight3 != -1)
	invertKey(keymaps[current_map]->highlight3);
    if (keymaps[current_map]->highlight4 != -1)
	invertKey(keymaps[current_map]->highlight4);
    return (0);
}



/*
**
**
**
*/
int last_key, keymap_changed = 0;
KeymapEntry *kentry;

void KbdWriteUP()
{
	if ( last_key) {
            KbdWrite(last_key,0);
                if ( ! keymap_changed) {
                        invertArea(kentry->ulx, kentry->uly, kentry->lrx, kentry->lry);
                        GrFlush();
                }
        }
}



static int
mouseClick(int xpos, int ypos)
{
    ExtEntry *eentry;
    // CLM KeymapEntry *kentry;
    int count, index, loop, maxkeys;

    maxkeys = keymaps[current_map]->maxkeys;

    // check for a keyclick
    count = keymap_changed = last_key = 0;
    do {
	// recover a pointer to this keymap entry
	kentry = keymaps[current_map]->keys + count;

	// skip disabled keymap entries
	if (kentry->keycode == -1)
	    continue;

	// check for pointer/key intersection
	if ((xpos >= kentry->ulx) &&
	    (xpos <= kentry->lrx) &&
	    (ypos >= kentry->uly) && (ypos <= kentry->lry)) {
	    // flash the key
	    invertArea(kentry->ulx, kentry->uly, kentry->lrx, kentry->lry);
	    GrFlush();
	    /* CLM usleep(FLASH_DELAY);
	    invertArea(kentry->ulx, kentry->uly, kentry->lrx, kentry->lry);
	    GrFlush();
	    * */

	    // standard keypress
	    if (kentry->keycode >= 0) {
		// scan extensions for keycode match
		/*if (keymaps[current_map]->maxexts > 0) {
		    loop = 0;
		    do {
			eentry = keymaps[current_map]->exts + loop;
			if (kentry->keycode == eentry->keycode) {
//				fprintf("nxkeyboard: special key => '%s'\n", eentry->keycode);
			    index = 0;
			    while (eentry->str[index]) {
					KbdWrite(eentry->str[index++], 1);
					KbdWrite(eentry->str[index++], 0);
				}
				invertArea(kentry->ulx, kentry->uly, kentry->lrx, kentry->lry);
	    		GrFlush();
			    return (0);
			}
		    } while (++loop < keymaps[current_map]->maxexts);
		}
*/
		KbdWrite(last_key = kentry->keycode, 1);

		if (old_map != -1) {
		    current_map = old_map;
		    old_map = -1;
		    updateWindow();
                    keymap_changed = 1;
		}
		return (0);
	    }
	    // temporary keymap change
	    else if ((kentry->keycode <= -2) && (kentry->keycode >= -9)) {
		loop = 0;
		do {
		    if (keymaps[loop] != NULL) {
			if (keymaps[loop]->mapid == kentry->keycode) {
			    old_map = current_map;
			    current_map = loop;
			    updateWindow();
                            keymap_changed = 1;
			}
		    }
		} while (++loop < MAX_KEYMAPS);
	    }
	    // permanent keymap change
	    else {
		loop = 0;
		do {
		    if (keymaps[loop] != NULL) {
			if (keymaps[loop]->mapid == kentry->keycode) {
			    current_map = loop;
			    old_map = -1;
			    updateWindow();
                            keymap_changed = 1;
			}
		    }
		} while (++loop < MAX_KEYMAPS);
	    }
	}
    } while (++count < maxkeys);

    return (0);
}



/*
**
**
**
*/
int
main(int argc, char *argv[])
{
    DIR *dir;
    GR_EVENT event;
    KeymapEntry *kentry;
    char buf[256], file[256];
    int count, keysize, loop, mapid, mapindex, parms, size;
    struct dirent *dentry;

    // check for usage
    if (argc == 1) {
	printf("\nnxkeyboard usage:\n");
	printf("  nxkeyboard -d /path/to/keymap/dir -m mapset\n\n");
	exit(1);
    }
    // open graphics
    if (GrOpen() < 0) {
	fprintf(stderr, "nxkeyboard: cannot open graphics\n");
	exit(1);
    }
    // open access to the keyboard
    if (KbdOpen() < 0) {
	fprintf(stderr, "nxkeyboard: cannot open kbd named pipe.\n");
	exit(1);
    }
    // recover the command line parameters
    parms = parseCommandLine(argc, argv);
    if (parms != 2) {
	fprintf(stderr, "nxkeyboard: incorrect command line parameters.\n");
	exit(1);
    }
    // clear keymap list
    count = 0;
    do {
	keymaps[count] = NULL;
	images[count] = 0;
    } while (++count < MAX_KEYMAPS);

    // open the specified keymap directory
    printf("opening directory %s\n", gMapDir);
    dir = opendir(gMapDir);
    if (dir == NULL) {
	fprintf(stderr, "nxkeyboard: unable to load keymaps.\n");
	exit(1);
    }
    // load keymaps
    count = 0;
    while (1) {
	// recover the next directory entry
	dentry = readdir(dir);
	if (dentry == NULL)
	    break;

	// skip current, parent, and sub directories
	if (strcmp(dentry->d_name, ".") == 0)
	    continue;
	if (strcmp(dentry->d_name, "..") == 0)
	    continue;

	// see if this entry matches
	strcpy(file, dentry->d_name);
	if (strncmp(file, gMapID, strlen(gMapID)) != 0)
	    continue;
	if (strstr(file, ".map") != 0)
	    continue;

	// match found - create keymap filename
	*(strstr(file, ".")) = 0x00;
	sprintf(buf, "%s/%s.map", gMapDir, file);
	keymaps[count] = keymapLoadMap(buf);
	if (keymaps[count] == NULL)
	    continue;

	// create imagemap filename
	sprintf(buf, "%s/%s.bmp", gMapDir, file);
	images[count] = GrLoadImageFromFile(buf, 0);
	++count;
    }
    closedir(dir);

    // scan all keymaps - find starting keymap and largest keysize
    mapid = -1000000;
    mapindex = -1;
    keysize = 0;
    count = 0;
    do {
	if (keymaps[count] != NULL) {
	    if ((keymaps[count]->mapid <= -10)
		&& (keymaps[count]->mapid > mapid)) {
		mapid = keymaps[count]->mapid;
		mapindex = count;
	    }

	    loop = 0;
	    do {
		kentry = keymaps[count]->keys + loop;
		size =
		    ((kentry->lrx - kentry->ulx) +
		     1) * ((kentry->lry - kentry->uly) + 1);
		if (size > keysize)
		    keysize = size;
	    } while (++loop < keymaps[count]->maxkeys);
	}
    } while (++count < MAX_KEYMAPS);

    // check for no maps
    if (mapindex < 0) {
	fprintf(stderr, "nxkeyboard: no maps or images.\n");
	return (0);
    }
    // allocate memory from the system
    pixels = (GR_PIXELVAL *) malloc(keysize * sizeof(GR_PIXELVAL));
    if (pixels == NULL) {
	fprintf(stderr, "nxkeyboard: not enough memory\n");
	return (0);
    }
    // set defaults
    current_map = mapindex;
    old_map = -1;
    extra_width = extra_height = -1;

    /* Set up the geometry to force flush with bottom left corner */
    sprintf(cpGeom, "%dX%d+0-0", keymaps[current_map]->width,
	    keymaps[current_map]->height);
    {
	int ii = 0;
	while (nxargs[ii].type != 0) {
	    if (nxargs[ii].type & nxtypeGEOMETRY) {
		nxargs[ii].defvalue = cpGeom;
		break;
	    }			/* end of if */
	    ii++;
	}			/* end of while */
    }				/* end of memory lifetime (ii) */

    // CLM w = nxCreateAppWindow(&argc, &argv, nxargs);
	w = GrNewWindowEx(GR_WM_PROPS_APPWINDOW | GR_WM_PROPS_NORESIZE | 
    			      /*GR_WM_PROPS_BORDER | */GR_WM_PROPS_NOFOCUS, 
    			      "nxkeyboard",GR_ROOT_WINDOW_ID,
							10, 10, 150, 100, BLACK);


    // enable specific window events
    GrSelectEvents(w,
		   GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_BUTTON_UP |
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);

    // display the window
    GrMapWindow(w);
    GrResizeWindow(w, keymaps[current_map]->width,
		   keymaps[current_map]->height);
    GrFlush();
    gc = GrNewGC();

    // event loop
    while (1) {
	GrGetNextEvent(&event);

	switch (event.type) {
	    // close the application
	case GR_EVENT_TYPE_CLOSE_REQ:
	    count = 0;
	    do {
		if (keymaps[count] != NULL)
		    keymapDeleteMap(keymaps[count]);
		if (images[count] != 0)
		    GrFreeImage(images[count]);
	    } while (++count < MAX_KEYMAPS);
	    GrClose();
	    exit(0);
	    break;

	    // redraw the window
	case GR_EVENT_TYPE_EXPOSURE:
	    updateWindow();
	    break;

	    // mouse click
	case GR_EVENT_TYPE_BUTTON_DOWN:
	    mouseClick(event.button.x, event.button.y);
	    break;
	case GR_EVENT_TYPE_BUTTON_UP:
		KbdWriteUP();
		break;
	}
    }
}
