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


#ifndef FILETOOLS_H
#define FILETOOLS_H


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



/*
**
** Global Variable Declarations
**
*/



/*
**
**  NAME: splitPath()
**
** USAGE: int splitPath(char *path, char *dir, char *file, char *ext);
**
** DESCR: This function will split the specified path into its directory,
**        filename, and extension components.
**
** PARMS: The "path" paramter is a pointer to a buffer that contains the
**        path to be split. The "dir" parameter is a pointer to a buffer
**        that will hold the directory result. The "file" parameter is
**        a pointer to a buffer that will hold the filename result. The
**        "ext" parameter is a pointer to a buffer that will hold the
**        file extension result.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        execution, a non-zero value is returned that describes the error.
**
*/
int splitPath(char *, char *, char *, char *);

#endif /* FILETOOLS_H */
