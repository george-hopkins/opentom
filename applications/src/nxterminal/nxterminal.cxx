/*
 * OpenTom nxterminal.cxx, by Clément GERARDIN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define MWINCLUDECOLORS
#include <microwin/nano-X.h>
#include "gterm.hpp"

#define TERM_WIDTH 40
#define TERM_HEIGHT 14

class NXTerminalWindow : public GTerm
{
    
    char screen[GT_MAXHEIGHT][GT_MAXWIDTH];
    int mfd, cursor_x, cursor_y, char_w, char_h, char_b, scrn_sx, scrn_sy, scrn_w, scrn_h, ox, oy, osx, osy;
    
    GR_WINDOW_ID wid;
    //GR_FONT_ID	font;
    GR_GC_ID gc;    
    
public:
        
        NXTerminalWindow(int width, int height) : GTerm(80, 24), scrn_w(width), scrn_h(height)  {
            GR_FONT_INFO finfo;
            GR_FONT_ID     font;
            scrn_sx = scrn_sy = osx, osy = mfd = 0;
            gc = GrNewGC();
            GrSetGCFont(gc, font=GrCreateFontEx(GR_FONT_SYSTEM_FIXED, 0, 0, NULL));
            GrGetFontInfo(font, &finfo);
            char_w = finfo.maxwidth;
            char_h = finfo.height;
            char_b = finfo.baseline;
            memset( screen, ' ', GT_MAXHEIGHT*GT_MAXWIDTH);
            
            wid = GrNewWindowEx (
               GR_WM_PROPS_APPFRAME | GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX
               | GR_WM_PROPS_NOBACKGROUND, "OpenTom NanoX Terminal",
               GR_ROOT_WINDOW_ID, 20, 20, char_w*TERM_WIDTH, char_h*TERM_HEIGHT, WHITE);

            GrSelectEvents (wid, GR_EVENT_MASK_EXPOSURE | 
                             GR_EVENT_MASK_KEY_UP | GR_EVENT_MASK_KEY_DOWN |
                             GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN | 
                             GR_EVENT_MASK_MOUSE_MOTION |
                             GR_EVENT_MASK_UPDATE | GR_EVENT_MASK_CLOSE_REQ);

            GrMapWindow (wid);
 
        }
    
    // manditory child-supplied functions
	virtual void DrawText(int fg_color, int bg_color, int flags, 
		int x, int y, int len, unsigned char *string) {

            char *dest = &screen[y][x];
            strncpy(dest, (char *)string, len);
            //printf("DrawText[%d,%d](%d:%d)=(%d)[%d]'%s'\n", x, y, fg_color,bg_color,len, *string, string);
        }

	virtual void DrawCursor(int fg_color, int bg_color, int flags, 
		int x, int y, unsigned char c) {
            cursor_x = x;
            cursor_y = y;
            // Scroll to allways see cursor
            if ( cursor_x < scrn_sx)  scrn_sx = cursor_x;
            else if ( cursor_x > scrn_sx + scrn_w) 
                scrn_sx = 1 + cursor_x - scrn_w;
            if ( cursor_y < scrn_sy)  scrn_sy = cursor_y;
            else if ( cursor_y > scrn_sy + scrn_h) 
                scrn_sy = 1 + cursor_y - scrn_h;
            
            GrFlushWindow(wid);
        }

	// optional child-supplied functions
	virtual void MoveChars(int sx, int sy, int dx, int dy, int w, int h) {
            for( int i = 0; i < h; i ++)
                for( int j = 0; j < w; j++)
                    screen[dy+i][dx+j] = screen[sy+i][sx+j];
            //printf("MoveChars (%d,%d)[%d,%d] =>(%d,%d)\n", sx, sy, w, h, dx, dy);
            GrFlushWindow(wid);
        }
        
	virtual void ClearChars(int bg_color, int x, int y, int w, int h) {
            for( int i = 0; i < h; i ++)
                for( int j = 0; j < w; j++)
                    screen[y+i][x+j] = ' ';
            
            //printf("ClearChars (%d,%d)[%d,%d]\n", x, y, w, h);
            GrFlushWindow(wid);
        }
        
/*        virtual void SendBack(char *data) { 
            int i;
            if (mfd > 0) i = write(mfd, data, strlen(data));
            if ( i < 0) { perror("write:"); exit(1); }
        } */

        int pts_slave(int mfd)
        {
                char pty_name[32];
                struct stat statbuf;
                int sfd;
                int fail = grantpt(mfd);
                if (fail) 
                        return -1;
                fail = unlockpt(mfd);
                if (fail) 
                        return -2;

                strncpy(pty_name, ptsname(mfd), 32);
                if (stat(pty_name, &statbuf) < 0)
                        return -3;

                sfd = open(pty_name, O_RDWR);
                return sfd;
        }
        
        
        void shell_fork() {
            mfd = getpt();

            if (mfd < 0) {
                    fprintf(stderr, "Can't open master pty\n");
                    exit(1);
            }

            int pid = fork();
            if (pid < 0) 
            {
                    fprintf(stderr, "Can't fork\n");
                    exit(1);
            }

            if (!pid) 
            { // slave process
                    int sfd = pts_slave(mfd);
                    close(mfd);

                    if (sfd < 0)
                    {
                            fprintf(stderr, "Can't open child (%d)\n", sfd);
                            exit(1);
                    }

                    if (setsid() < 0) 
                            fprintf(stderr, "Could not set session leader\n");

                    dup2(sfd, 0);
                    dup2(sfd, 1);
                    dup2(sfd, 2);
                    if (sfd > 2)
                            close(sfd);

                    execl("/bin/sh", "/bin/sh", NULL);
                    perror("/bin/sh");
                    exit(1);
            } // end of slave process
                // else the master process...
            clear_mode_flag(GTerm::DEFERUPDATE);
#ifdef TEXTONLY
            clear_mode_flag(GTerm::TEXTONLY);
#else       
            set_mode_flag(GTerm::TEXTONLY);
#endif
            set_mode_flag(GTerm::NOEOLWRAP);   // disable line wrapping
            clear_mode_flag(GTerm::LOCALECHO); // disable local echo
            fcntl(mfd, F_SETFL, O_NONBLOCK);
        }
        
        int shell_run() {
            
            shell_fork();
            GR_EVENT ev;
            char buff[1024];
            int len, _w, _h;
            
            do {
		GrGetNextEventTimeout(&ev, 100);
		switch ( ev.type) {
                    case GR_EVENT_TYPE_EXPOSURE:
                        GrSetGCBackground(gc, BLACK);
                     
                        _h = (scrn_h >= GT_MAXWIDTH) ? scrn_h : scrn_h+1;
                        _w = (scrn_w >= GT_MAXWIDTH) ? scrn_w : scrn_w+1;
                        for(int i =  0; i < _h; i ++) {
#ifdef TEXTONLY
                            char *s;
                            int len = strlen(s=&screen[i+scrn_sy][scrn_sx]);
                            GrSetGCForeground(gc, GREEN);
                            GrText(wid, gc, 0, i*char_h+char_b, &screen[i+scrn_sy][scrn_sx], len,0);
                            GrSetGCForeground(gc, BLACK);
                            GrFillRect(wid, gc, len*char_w, i*char_h, (scrn_w-len)*char_w, char_h);
#else
                            GrText(wid, gc, 0, i*char_h+char_b, &screen[i+scrn_sy][scrn_sx], _w,0);
#endif                            
                        }
                        
                        GrSetGCForeground(gc, WHITE);
                        GrRect(wid, gc, (cursor_x-scrn_sx)*char_w, (cursor_y-scrn_sy)* char_h, char_w, char_h);
                        break;
                        
                    case GR_EVENT_TYPE_MOUSE_MOTION:
                        if ( osx < 0) { // first time
                            ox = ev.button.x;
                            oy = ev.button.y;
                            osx = scrn_sx;
                            osy = scrn_sy;
                            break;
                        }
                        if ( ev.mouse.buttons) {
                            /*printf("Motion(%d,%d)[%d =? %d] <%d, %d>\n", ev.button.x, ev.button.y, 
                                ev.button.wid, wid, ev.button.rootx, ev.button.rooty);*/
                            scrn_sx = osx + (ox - ev.button.x) / char_w;
                            scrn_sy = osy + (oy - ev.button.y) / char_h;
                            //printf("** Motion(%d,%d):(%d,%d) = (%d, %d) ==> ", osx, osy, ox, oy, scrn_sx, scrn_sy);
                            if ( scrn_sx < 0) scrn_sx = 0;
                            if ( scrn_sy < 0) scrn_sy = 0;
                            if ( scrn_sx + scrn_w > 80) scrn_sx = 80 - scrn_w;
                            if ( scrn_sy + scrn_h > 24) scrn_sy = 24 - scrn_h;
                            //printf(" (%d, %d)\n", scrn_sx, scrn_sy);
                            GrFlushWindow(wid);
                            
                        }
                        break;
                        
                    case GR_EVENT_TYPE_BUTTON_UP:         
                        osx = -1;
                        break;
                        
                    case GR_EVENT_TYPE_KEY_DOWN:
                        switch ( ev.keystroke.ch) {
                            case MWKEY_HOME: 
                                write( mfd, "\e[OH", 4); break;
                            case MWKEY_END: 
                                write( mfd, "\e[OF", 4); break;
                            case MWKEY_UP: 
                                write( mfd, "\e[A", 3); break;
                            case MWKEY_DOWN: 
                                write( mfd, "\e[B", 3); break;
                            case MWKEY_RIGHT: 
                                write( mfd, "\e[C", 3); break;
                            case MWKEY_LEFT:
                                write( mfd, "\e[D", 3); break;
                            case MWKEY_PAGEUP: 
                                write( mfd, "\e[5~", 4); break;
                            case MWKEY_PAGEDOWN: 
                                write( mfd, "\e[6~", 4); break;                                
                            default: 
                                write( mfd, &ev.keystroke.ch, 1);
                        }
                        
                        break;
                        
                    case GR_EVENT_TYPE_UPDATE:
                        scrn_w = ev.update.width / char_w;
                        if (scrn_w > GT_MAXWIDTH) scrn_w = GT_MAXWIDTH;
                        if (scrn_h > GT_MAXHEIGHT) scrn_w = GT_MAXHEIGHT;
                        scrn_h = ev.update.height / char_h;
                        if ( scrn_sx + scrn_w > 80) scrn_sx = 80 - scrn_w;
                        if ( scrn_sy + scrn_h > 24) scrn_sy = 24 - scrn_h;
                        GrFlushWindow(wid);
                        break;
                        
                    case GR_EVENT_TYPE_CLOSE_REQ: 
                        return 0;
                }
                
                
                // Read from shell
                while( (len = read(mfd, buff, 1024)) > 0) {
                    ProcessInput(len, (unsigned char *)buff);
                    GrFlushWindow(wid);
                }
                
            } while(1);
        }
};

int main(int argc, char **argv)
{       
    if (GrOpen() < 0) exit(1);
    NXTerminalWindow *term = new NXTerminalWindow(40, 14);
    return term->shell_run();
}
