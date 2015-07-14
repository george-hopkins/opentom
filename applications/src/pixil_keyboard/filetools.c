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
#include <string.h>


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



/*
**
** Local Variable Declarations
**
*/



/*
**
** This function will split the specified path into its directory,
** filename, and extension components.
**
** The "path" paramter is a pointer to a buffer that contains the
** path to be split. The "dir" parameter is a pointer to a buffer
** that will hold the directory result. The "file" parameter is
** a pointer to a buffer that will hold the filename result. The
** "ext" parameter is a pointer to a buffer that will hold the
** file extension result.
**
** If successful, '0' is returned. If an error occurs during function
** execution, a non-zero value is returned that describes the error.
**
*/
int
splitPath(char *path, char *dir, char *file, char *ext)
{
    char buf[1024];
    int count, index;

    // clear all result buffers
    *dir = *file = *ext = 0x00;

    // check for null or empty path
    if (path == NULL)
	return (0);
    if (*path == 0x00)
	return (0);

    // recover the length of the path
    strcpy(buf, path);
    index = strlen(buf) - 1;

    // locate the end of the directory
    count = index;
    while (1) {
	if (count < 0)
	    break;
	if (buf[count] == '/')
	    break;
	--count;
    }
    if (count >= 0) {
	strcpy(dir, buf);
	*(dir + ++count) = 0x00;
    } else
	count = 0;

    // check for directory only
    if (buf[count] == 0x00)
	return (0);

    // locate the extension
    while (1) {
	if (index < count)
	    break;
	if (buf[index] == '.')
	    break;
	--index;
    }

    // filename with no '.' at all
    if (index < count)
	strcpy(file, buf + count);

    // filename with '.' at beginning
    else if (index == count)
	strcpy(file, buf + count);

    // filename with extension
    else {
	strcpy(ext, buf + index + 1);
	buf[index] = 0x00;
	strcpy(file, buf + count);
    }

    // exit with no errors
    return (0);
}
