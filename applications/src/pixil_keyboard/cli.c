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



/*
**
** Global Variable Declarations
**
*/
char gMapID[256];
char gMapDir[256];



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
    char *parm_flag;		// parameter keyword
    void *parm_result;		// where to place parameter value
    int parm_type;		// 0=integer | 1=string | 2=bool
}
ParmEntry;



/*
**
** Local Variable Declarations
**
*/
ParmEntry parm_list[] = {
    {"-m", gMapID, 1},
    {"-d", gMapDir, 1},
    {NULL, NULL, 0}
};



/*
**
** This function will parse the list of command line arguments
** and recover their associated information.
**
** The "argc" parameter specifies the number of arguments found in
** the argv list. The "argv" parameter is a pointer to the array
** of command line arguments.
**
** If successful, the number of command line arguments parsed is
** is returned. If an error occurs during function , a negative
** value is returned.
**
*/
int
parseCommandLine(int argc, char **argv)
{
    int count, len = 0, loop, total;
    char *arg;

    // initialize any bools and strings in the list
    loop = 0;
    while (1) {
	if (parm_list[loop].parm_flag == NULL)
	    break;
	if (parm_list[loop].parm_type == 1)
	    *((char *) parm_list[loop].parm_result) = 0x00;
	if (parm_list[loop].parm_type == 2)
	    *((int *) parm_list[loop].parm_result) = 0;
	++loop;
    }

    // check for empty list
    if (argc <= 1)
	return (0);

    // locate and process all command line flags
    total = 0;
    count = 1;
    do {
	// recover and verify the next entry
	arg = *(argv + count);
	if (*arg != '-')
	    continue;

	// scan list of valid arguments
	loop = 0;
	while (1) {
	    if (parm_list[loop].parm_flag == NULL)
		break;
	    len = strlen(parm_list[loop].parm_flag);

	    if (strncmp(arg, parm_list[loop].parm_flag, len) == 0)
		break;
	    ++loop;
	}

	// check for no match in list
	if (parm_list[loop].parm_flag == NULL)
	    continue;

	// handle integer parameter
	if (parm_list[loop].parm_type == 0) {
	    if (strlen(arg) == len) {
		if (++count >= argc)
		    continue;
		*((int *) parm_list[loop].parm_result) =
		    atoi(*(argv + count));
	    } else
		*((int *) parm_list[loop].parm_result) = atoi(arg + len);
	}
	// handle string parameter
	else if (parm_list[loop].parm_type == 1) {
	    if (strlen(arg) == len) {
		if (++count >= argc)
		    continue;
		strcpy((char *) parm_list[loop].parm_result, *(argv + count));
	    } else
		strcpy((char *) parm_list[loop].parm_result, arg + len);
	}
	// handle boolean parameter
	else if (parm_list[loop].parm_type == 2)
	    *((int *) parm_list[loop].parm_result) = 1;

	// bump the keyword counter
	++total;

    } while (++count < argc);

    // exit with no errors
    return (total);
}
