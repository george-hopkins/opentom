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



/* Copyright (C) 2000 by VTech Informations LTD.
 * Vladimir Cotfas <vladimircotfas@vtech.ca> Aug 31, 2000
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <microwin/nano-X.h>

#define KBDPIPE		0	/* =1 to use named pipe for soft kbd */

#if KBDPIPE
static char KBD_NAMED_PIPE[] = "/tmp/.nano-X-softkbd";
static int kbd_fd = -1;

int
KbdOpen(void)
{
    if (kbd_fd != -1)
	close(kbd_fd);

    if ((kbd_fd = open(KBD_NAMED_PIPE, O_WRONLY)) < 0)
	return -1;

    return kbd_fd;
}

void
KbdClose(void)
{
    if (kbd_fd >= 0) {
	close(kbd_fd);
	kbd_fd = -1;
    }
}

int
KbdWrite(int c)
{
    char cc = c & 0xff;

    return write(kbd_fd, &cc, 1);
}

#else /* !KBDPIPE */

int
KbdOpen(void)
{
    return 0;
}

void
KbdClose(void)
{
}

int
KbdWrite(int c, int pushed)
{
    GR_WINDOW_ID win = GrGetFocus();

    /* FIXME: modifiers are incorrect */
//    fprintf(stderr, "nxkeyboard.KbdWrite(%d == '%c', %d)\n", c, c, pushed);
    GrInjectKeyboardEvent(win, c, 0, c, pushed);
    //GrInjectKeyboardEvent(win, c, 0, 0, 0);
    return 1;
}
#endif /* KBDPIPE */
