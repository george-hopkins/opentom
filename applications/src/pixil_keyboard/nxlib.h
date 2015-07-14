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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef _NXLIB_H_
#define _NXLIB_H_

#include "microwin/nano-X.h"

/* PDA style, thin border, maximized, caption/closebox*/
#define STYLE_PDA		(/*GR_WM_PROPS_BORDER |*/ \
				GR_WM_PROPS_MAXIMIZE |\
				GR_WM_PROPS_CAPTION |\
				GR_WM_PROPS_NOMOVE |\
				GR_WM_PROPS_CLOSEBOX)

/* Webpad style, thick border, caption/closebox*/
#define STYLE_WEBPAD		(GR_WM_PROPS_APPFRAME | \
				 GR_WM_PROPS_BORDER | \
				GR_WM_PROPS_CAPTION |\
				GR_WM_PROPS_CLOSEBOX)

/* dbl linked list data structure*/

typedef struct _nxlist
{				/* LIST must be first decl in struct */
    struct _nxlist *next;	/* next item */
    struct _nxlist *prev;	/* previous item */
}
NXLIST, *PNXLIST;

/* dbl linked list head data structure*/

typedef struct _nxlisthead
{
    struct _nxlist *head;	/* first item */
    struct _nxlist *tail;	/* last item */
}
NXLISTHEAD, *PNXLISTHEAD;

void *nxItemAlloc(unsigned int size);
void nxListAdd(PNXLISTHEAD pHead, PNXLIST pItem);
void nxListInsert(PNXLISTHEAD pHead, PNXLIST pItem);
void nxListInsertAfter(PNXLISTHEAD pHead, PNXLIST pItem, PNXLIST pPrev);

void nxListRemove(PNXLISTHEAD pHead, PNXLIST pItem);

#define nxItemNew(type)	((type *)nxItemAlloc(sizeof(type)))
#define nxItemFree(ptr)	free((void *)ptr)

/* field offset*/
#define NXITEM_OFFSET(type, field)    ((long)&(((type *)0)->field))

/* return base item address from list ptr*/
#define nxItemAddr(p,type,list)	((type *)((long)p - NXITEM_OFFSET(type,list)))

/* calculate container size from client style/size*/
void nxCalcNCSize(GR_WM_PROPS style, GR_SIZE wClient,
		  GR_SIZE hClient, GR_COORD * xCliOffset,
		  GR_COORD * yCliOffset, GR_SIZE * wContainer,
		  GR_SIZE * hContainer);

/* calculate client size from container style/size*/
void nxCalcClientSize(GR_WM_PROPS style, GR_SIZE wContainer,
		      GR_SIZE hContainer, GR_COORD * xCliOffset,
		      GR_COORD * yCliOffset, GR_SIZE * wClient,
		      GR_SIZE * hClient);

/* utility routines*/
void strzcpy(char *dst, char *src, int count);

/* nxgeom.c - geometry parser*/
int nxGetGeometry(GR_CHAR * user_geom, GR_CHAR * def_geom,
		  GR_WM_PROPS style, GR_COORD * x_return,
		  GR_COORD * y_return, GR_SIZE * width_return,
		  GR_SIZE * height_return);

/* nxutil.c - utility routines*/

/* return default window decoration style*/
GR_WM_PROPS nxGetDefaultWindowStyle(void);

/* application initialization defines for use with nxCreateAppWindow*/
typedef struct
{
    int type;			/* init type */
    char *optname;		/* option name */
    char *defvalue;		/* default value */
}
nxARGS;

GR_WINDOW_ID nxCreateAppWindow(int *aac, char ***aav, nxARGS * alist);

/* command line options*/
#define nxstrTITLE		"-title"
#define nxstrGEOMETRY		"-geom"
#define nxstrBACKGROUND		"-background"
#define nxstrSTYLE		"-style"

#define nxtypeTITLE		1
#define nxtypeGEOMETRY		2
#define nxtypeBACKGROUND	3
#define nxtypeSTYLE		4
#define nxSTR			0x0000
#define nxINT			0x0100

#define nxTITLE(title)	    {nxtypeTITLE|nxSTR,nxstrTITLE,title}
#define nxGEOMETRY(geom)    {nxtypeGEOMETRY|nxINT,nxstrGEOMETRY,(char *)(geom)}
#define nxBACKGROUND(color) {nxtypeBACKGROUND|nxINT,nxstrBACKGROUND,\
				(char *)(color)}
#define nxSTYLE(style)	    {nxtypeSTYLE|nxINT,nxstrSTYLE,(char *)(style)}
#define nxEND		    {0, NULL, NULL}

/* iniread.c*/
int IniGetString(char *section, char *key, char *defval, char *retbuf,
		 int bufsiz, char *inifile);
void IniEnumKeyValues(char *buf, void (*pfn) (char *, char *, int), int data);


#ifdef NOTUSED
/* Configuration file routines */


#define NX_CONFIG_MAIN  0
#define NX_CONFIG_ASSOC 1
#define NX_CONFIG_COUNT 2

int nxLoadConfigFile(char *filename, int index);
char *nxGetConfigFile(int index);
void nxFreeConfigFiles();

/* directory search defines*/

/* enumeration function is called until 0 value returned*/
typedef int (*nxEnumProc) (char *path, int mode, void *);

#define NXDIR_FILE	01	/* regular files */
#define NXDIR_EXEC	02	/* executable files */

/* search passed directory for files according to flag options*/
int nxSearchDir(char *dir, char *pattern, int flags, nxEnumProc cbProc,
		void *data);

#endif

#endif /* _NXLIB_H_ */
