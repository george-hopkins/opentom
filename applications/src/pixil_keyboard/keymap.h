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


#ifndef KEYMAP_H
#define KEYMAP_H


/*
**
** Imported "Include" Files
**
*/



/*
**
** Global Constant Definitions
**
*/
#define MAXEXTLEN 16



/*
**
** Global Enumeration Definitions
**
*/



/*
**
** Global Structure Definitions
**
*/
typedef struct
{
    int ulx;
    int uly;
    int lrx;
    int lry;
    int keycode;
}
KeymapEntry;

typedef struct
{
    int keycode;
    char str[MAXEXTLEN];
}
ExtEntry;

typedef struct
{
    int mapid;
    int highlight1;
    int highlight2;
    int highlight3;
    int highlight4;
    int width;
    int height;
    KeymapEntry *keys;
    unsigned short maxkeys;
    ExtEntry *exts;
    unsigned short maxexts;
}
KeymapHandle;



/*
**
** Global Variable Declarations
**
*/



/*
**
**  NAME: keymapLoadMap()
**
** USAGE: KeymapHandle *keymapLoadMap(char *file);
**
** DESCR: This function will load the specified keymap.
**
** PARMS: The "file" parameter is a pointer to the buffer that contains the
**        path and filename.
**
** RETRN: If successful, a pointer to a KeymapHandle is returned. If an error
**        occurs during function exeuction, NULL is returned.
**
*/
KeymapHandle *keymapLoadMap(char *);



/*
**
**  NAME: keymapDeleteMap()
**
** USAGE: int keymapDeleteMap(KeymapHandle *handle);
**
** DESCR: This function will delete a KeymapHandle that was previously
**        created by a successful call to "keymapLoadHandle()".
**
** PARMS: The "handle" parameter is a pointer to a KeymapHandle.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        execution, a non-zero value is returned that describes the error.
**
*/
int keymapDeleteMap(KeymapHandle *);

#endif /* KEYMAP_H */
