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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keymap.h"



/*
**
** Local Constant Definitions
**
*/



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
typedef struct
{
    char oldchr;
    unsigned char newchr;
}
EscapeCodeEntry;



/*
**
** Local Variable Declarations
**
*/
EscapeCodeEntry esc_list[] = {
    {'[', 27},			// escape
    {'b', 8},			// backspace
    {'f', 12},			// formfeed
    {'n', 10},			// newline
    {'r', 13},			// carriage return
    {'t', 9},			// horizontal tab
    {'v', 11},			// vertical tab
    {92, 92},			// backslash
    {0, 0}
};



/*
**
**
**
*/
static void
processEscapeCodes(char *src, char *dst)
{
    int count, index, loop;

    count = index = 0;
    while (1) {
	// check for end of source
	if (*(src + count) == 0x00)
	    break;

	// check for a full destination buffer
	if (index >= (MAXEXTLEN - 1))
	    break;

	// handle escape sequence
	if (*(src + count) == 92) {
	    loop = 0;
	    while (1) {
		if (esc_list[loop].oldchr == 0) {
		    ++count;
		    break;
		}
		if (esc_list[loop].oldchr == *(src + count + 1)) {
		    *(dst + index++) = esc_list[loop].newchr;
		    count += 2;
		    break;
		}
		++loop;
	    }
	}
	// handle regular character
	else
	    *(dst + index++) = *(src + count++);
    }

    // terminate destination and exit
    *(dst + index) = 0x00;
}



/*
**
** This function will load the specified keymap.
**
** The "file" parameter is a pointer to the buffer that contains the
** path and filename.
**
** If successful, a pointer to a KeymapHandle is returned. If an error
** occurs during function exeuction, NULL is returned.
**
*/
KeymapHandle *
keymapLoadMap(char *file)
{
    FILE *fp;
    ExtEntry *eentry;
    KeymapEntry *kentry;
    KeymapHandle *handle;
    char buf[256], ext[256];

    // open the mapfile
    if ((fp = fopen(file, "r")) == NULL)
	return (NULL);

    // allocate memory from the system
    handle = (KeymapHandle *) malloc(sizeof(KeymapHandle));
    if (handle == NULL) {
	fclose(fp);
	return (NULL);
    }
    handle->keys = NULL;
    handle->maxkeys = 0;
    handle->exts = NULL;
    handle->maxexts = 0;

    // load the keymap file
    while (1) {
	// read the next line from the file and strip the return
	if (fgets(buf, 256, fp) == NULL)
	    break;
	buf[strlen(buf) - 1] = 0x00;

	// check for empty or comment lines
	if (buf[0] == 0x00)
	    continue;
	if (buf[0] == '*')
	    continue;

	// handle parameter entry
	if (strncmp(buf, "parms:", 6) == 0) {
	    sscanf(buf + 6,
		   "%d,%d,%d,%d,%d,%d,%d",
		   &handle->mapid,
		   &handle->highlight1,
		   &handle->highlight2,
		   &handle->highlight3,
		   &handle->highlight4, &handle->width, &handle->height);
	}
	// handle keymap entry
	else if (strncmp(buf, "keymap:", 7) == 0) {
	    if (handle->keys == NULL) {
		handle->keys = (KeymapEntry *) malloc(sizeof(KeymapEntry));
		handle->maxkeys = 0;
	    } else {
		handle->keys =
		    (KeymapEntry *) realloc(handle->keys,
					    (handle->maxkeys +
					     1) * sizeof(KeymapEntry));
	    }
	    if (handle->keys == NULL) {
		free(handle);
		fclose(fp);
		return (NULL);
	    }
	    // add the keymap to the list
	    kentry = handle->keys + handle->maxkeys;
	    sscanf(buf + 7,
		   "%d,%d,%d,%d,%d",
		   &kentry->keycode,
		   &kentry->ulx, &kentry->uly, &kentry->lrx, &kentry->lry);
	    handle->maxkeys += 1;
	}
	// handle multichar entry
	else if (strncmp(buf, "multi:", 6) == 0) {
	    if (handle->exts == NULL) {
		handle->exts = (ExtEntry *) malloc(sizeof(ExtEntry));
		handle->maxexts = 0;
	    } else {
		handle->exts =
		    (ExtEntry *) realloc(handle->exts,
					 (handle->maxexts +
					  1) * sizeof(ExtEntry));
	    }
	    if (handle->exts == NULL) {
		free(handle);
		fclose(fp);
		return (NULL);
	    }
	    // add the extension to the list
	    eentry = handle->exts + handle->maxexts;
	    eentry->str[0] = 0x00;
	    sscanf(buf + 6, "%d", &eentry->keycode);
	    strcpy(ext, strstr(buf, ",") + 1);
	    processEscapeCodes(ext, eentry->str);
	    handle->maxexts += 1;
	}
    }

    // do houskeeping
    fclose(fp);

    // return result and exit with no errors
    return (handle);
}



/*
**
** This function will delete a KeymapHandle that was previously
** created by a successful call to "keymapLoadHandle()".
**
** The "handle" parameter is a pointer to a KeymapHandle.
**
** If successful, '0' is returned. If an error occurs during function
** execution, a non-zero value is returned that describes the error.
**
*/
int
keymapDeleteMap(KeymapHandle * handle)
{
    if (handle->keys)
	free(handle->keys);
    if (handle->exts)
	free(handle->exts);
    free(handle);
    return (0);
}
