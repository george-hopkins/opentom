//	imgv, a simple SDL-based image viewer for the Ben Nanonote
//	Version 0.4.0
//	Last edited by Fernando Carello <fcarello@libero.it> 2010-05-24
//	Last edited by Niels Kummerfeldt <niels.kummerfeldt@tuhh.de> 2010-10-19
//	Last edited by Clement GERARDIN <opentom@free.fr> 2014-03-18
//
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_timer.h>
#include <SDL/SDL_video.h>
#include "SDL_rotozoom.h"

#define TRUE 1
#define FALSE 0
#define SCREENWIDTH scrWidth
#define SCREENHEIGHT scrHeight
#define SCREENBPP 32
#define SMOOTHING_OFF   0
#define SMOOTHING_ON    1
#define PANSTEP 40
#define ZOOMSTEP 1.2
#define SLIDESHOWTIMEOUT 1000 * 5
#define VERSION "0.3.0"

int showFileName = 0, runSlideShow = 0;
int scrWidth = 320;
int scrHeight = 240;
int winScrWidth, winScrHeight;
int currentImageNumber = 1;

#define MAX_FILE_LIST 4096
char *file_list[MAX_FILE_LIST];
int nb_file_in_list = 0;

int getFileList(char *path)
{  
    int nbf = 1;
    struct dirent *entree;
    DIR *rep;
		
    if (chdir(path)) { perror(path); return 1; }
    
    rep = opendir(".");
    if ( rep != NULL) {
        while ((nbf<MAX_FILE_LIST) && (entree = readdir(rep)))
                if ( entree->d_type == DT_REG)
                    file_list[nbf++] = strdup(entree->d_name);
        closedir(rep);
        
    } else perror("rep");
    
    return nb_file_in_list = nbf;
}


void setTitle()
{
    char buff[128];
    if ( ! showFileName)
    {
        snprintf(buff, 127, "SDLviewer %s %s", runSlideShow?">>":"--", file_list[currentImageNumber]);
        SDL_WM_SetCaption( buff, NULL);
    }
}



void quit()
{
    TTF_Quit();
    SDL_Quit();

    exit(1);
}

SDL_Surface *initScreen()
{
    // Initialize the SDL library 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) 
    {
        fprintf(stderr,	"\n Couldn't initialize SDL: %s\n\n", SDL_GetError());
        quit();
    }

    // Set video mode
    SDL_Surface *screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, SCREENBPP,
                                           SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);
    if (screen == (SDL_Surface *) (NULL)) 
    {
        fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n\n", SCREENWIDTH, SCREENHEIGHT, SCREENBPP, SDL_GetError());
        quit();
    }

    // Can't stand the useless arrow... Ben has no pointing device
    SDL_ShowCursor(SDL_DISABLE);	

    TTF_Init();

    return screen;
}

int found = 0;
SDL_Surface *loadImage(int *number)
{
    // Load Picture
    
    SDL_Surface *tmp = 0;
    
    do 
    {
        printf("try[%d]='%s'\n", *number, file_list[*number]);
        if (file_list[*number]) tmp = IMG_Load(file_list[*number]);
        if (!tmp && file_list[*number]) 
        {
                free(file_list[*number]);
                file_list[*number] = NULL;
        }
        
        if ( ! tmp) {
            (*number)++;
            if ( (*number) >= nb_file_in_list)
                    if ( ! found) {
                        // no image viewable in this directory
                        fprintf(stderr, "Could not open any image in this directory.");
                        exit(1);

                    } else {
                            (*number) = 1;
                            found = 0;
                    }
        }
        
    } while ( ! tmp);
    found = 1;

    // Auto rotate image to fit screen
    //if (tmp->w > SCREENWIDTH || tmp->h > SCREENHEIGHT) {
    //    if (tmp->h > tmp->w * 1.1) {
    //        SDL_Surface *t = rotateSurface90Degrees(tmp, 3);
    //        SDL_FreeSurface(tmp);
    //        tmp = t;
    //    }
    //}

    // Convert picture in same format as video framebuffer, to optimize blit performances
    SDL_Surface *picture = SDL_DisplayFormat(tmp);
    if (picture == (SDL_Surface *) (NULL)) 
    {
        fprintf(stderr, "\n Internal error from DisplayFormat\n\n");
        quit();
    }
    SDL_FreeSurface(tmp);
    
    if ( ! showFileName ) setTitle();
    
    return picture;
}

void pan(SDL_Surface *image, SDL_Rect *pos, int dx, int dy)
{
    if (image->w > SCREENWIDTH) {
        pos->x += dx;
        if (pos->x < 0) {
            pos->x = 0;
        }
        if (pos->x >= image->w - SCREENWIDTH) {
            pos->x = (Sint16) (image->w - SCREENWIDTH);
        }
    } else {
        pos->x = 0;
    }
    if (image->h > SCREENHEIGHT) {
        pos->y += dy;
        if (pos->y < 0) {
            pos->y = 0;
        }
        if (pos->y >= image->h - SCREENHEIGHT) {
            pos->y = (Sint16) (image->h - SCREENHEIGHT);
        }
    } else {
        pos->y = 0;
    }
}

SDL_Surface *zoomIn(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    *scale *= ZOOMSTEP;

    SDL_Surface *result = (SDL_Surface *)zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    pos->x *= ZOOMSTEP;
    int dx = SCREENWIDTH * (ZOOMSTEP-1) * 0.5;
    pos->y *= ZOOMSTEP;
    int dy = SCREENHEIGHT * (ZOOMSTEP-1) * 0.5;
    pan(result, pos, dx, dy);

    return result;
}

SDL_Surface *zoomOut(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    *scale /= ZOOMSTEP;

    SDL_Surface *result = (SDL_Surface *)zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    pos->x += SCREENWIDTH * (1-ZOOMSTEP) * 0.5;
    pos->x /= ZOOMSTEP;
    pos->y += SCREENHEIGHT * (1-ZOOMSTEP) * 0.5;
    pos->y /= ZOOMSTEP;
    pan(result, pos, 0, 0);

    return result;
}

SDL_Surface *zoomFit(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    pos->x = 0;
    pos->y = 0;
    double scale_x = (double) (SCREENWIDTH) / (double) (image->w);
    double scale_y = (double) (SCREENHEIGHT) / (double) (image->h);
    if (scale_y < scale_x) {
        *scale = scale_y;
    } else {
        *scale = scale_x;
    }

    SDL_Surface *result = (SDL_Surface *)zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    return result;
}

SDL_Surface *zoom100(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    SDL_Surface *result = SDL_ConvertSurface(image, image->format, image->flags);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from ConvertSurface()\n\n");
        quit();
    }

    if (*scale < 1.0) {
        pos->x /= *scale;
        pos->y /= *scale;
        pos->x -= SCREENWIDTH * (1-(1 / *scale)) * 0.5;
        pos->y -= SCREENHEIGHT * (1-(1 / *scale)) * 0.5;
    } else {
        pos->x += SCREENWIDTH * (1-*scale) * 0.5;
        pos->y += SCREENHEIGHT * (1-*scale) * 0.5;
        pos->x /= *scale;
        pos->y /= *scale;
    }
    pan(result, pos, 0, 0);
    *scale = 1.0;

    return result;
}

SDL_Surface *drawFileName(char *filename, TTF_Font *font, int slideShow)
{
    if(font) {
        SDL_Color foregroundColor = { 0, 0, 0, 0 }; 
        SDL_Color backgroundColor = { 200, 200, 200, 0 };

        char text[strlen(filename)+4];
        strcpy(text, filename);
        if (slideShow) {
            strcat(text, " >>");
        }
        return TTF_RenderText_Shaded(font, text, foregroundColor, backgroundColor);
    }
    return NULL;
}

void drawImage(SDL_Surface *image, SDL_Rect *pos, SDL_Surface *screen, SDL_Surface *filename)
{
    SDL_FillRect(screen, (SDL_Rect *) NULL, 0);	// draw background color (black)

    SDL_Rect screenPos;
    if (image->w < SCREENWIDTH) {
        screenPos.x = (SCREENWIDTH - image->w) / 2;
    } else {
        screenPos.x = 0;
    }
    if (image->h < SCREENHEIGHT) {
        screenPos.y = (SCREENHEIGHT - image->h) / 2;
    } else {
        screenPos.y = 0;
    }
    SDL_BlitSurface(image, pos, screen, &screenPos); 

    if(filename) {
        SDL_Rect textLocation = { 0, 0, 0, 0 };
        if (filename->w > SCREENWIDTH) {
            textLocation.x = SCREENWIDTH - filename->w;
        }
        SDL_BlitSurface(filename, NULL, screen, &textLocation);
    }

    SDL_Flip(screen);
}

Uint32 timerCallback(Uint32 interval, void *param)
{
    param = NULL;
    SDL_Event event;
    SDL_KeyboardEvent keyEvent;

    keyEvent.type = SDL_KEYDOWN;
    keyEvent.keysym.unicode = 0;
    keyEvent.keysym.scancode = 0;
    keyEvent.keysym.mod = 0;
    keyEvent.keysym.sym = SDLK_n;

    event.type = SDL_KEYDOWN;
    event.key = keyEvent;

    SDL_PushEvent(&event);

    return interval;
}


int lastW, lastH;
SDL_Surface *toggleFullScreen()
{
    SDL_Surface *screen; 
    
    if ( ! showFileName) {
        winScrWidth = scrWidth;
        winScrHeight = scrHeight;
        SDL_Rect **r= SDL_ListModes(NULL, SDL_FULLSCREEN);
        screen = SDL_SetVideoMode( (*r)[0].w, (*r)[0].h, 0, SDL_FULLSCREEN);
    } else {
        screen = SDL_SetVideoMode(scrWidth=winScrWidth, scrHeight=winScrHeight, SCREENBPP,
                    SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);
    }
    showFileName ^= 1;
    setTitle();
    
    return screen;
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen      = NULL,
                *image       = NULL,
                *scaledImage = NULL,
                *name        = NULL;
    SDL_Rect     picturePortion;
    TTF_Font    *font = NULL;
    double       scale = 1.0;
    int          opt, i, /*currentImageNumber = 1,*/
    //             showFileName = TRUE,
    //             runSlideShow = FALSE,
                 isRunning = TRUE;
    SDL_TimerID  slideShowTimer = 0;

    TTF_Init();
    
    // Process command line
    while ((opt = getopt(argc, argv, "d:f:")) != -1) {
        switch (opt) {
               case 'd':
                   getFileList( optarg);
                   break;
               case 'f':
                   font = TTF_OpenFont(optarg, 11);
                   break;
               default: /* '?' */
                   fprintf(stderr,  "\n"
            " imgv v%s. Syntax: imgv [ -f <font.ttf> ] [ -d path ] [ <image files> ]\n\n"
            " Screen control (see below):\n"
            " a | b | c\n"
            "----------\n"
            " d | e | f\n"
            "----------\n"
            " g | h | i\n\n"               
            " Hotkeys:\n"
            " (e)x2 toggle full screen\n"
            " 'f' (e) fit to screen\n"
            " 'z' (g) zoom at pixel level\n"
            " 'i' (h) zoom in  'o' (b) zoom out\n"
            " 'l' (a) rotate left  'r' (c) rotate right\n"
            " 'n' (f) next image  'p' (d) previous image\n"
            " 'd' show / hide file name\n"
            " 's' (i) start / stop slide show\n"
            " 'arrows' (drag) pan  'ESC' quit\n\n", VERSION);
                  
                   exit(EXIT_FAILURE);
        }
    }

    if( ! font) {
        font = TTF_OpenFont("/mnt/sdcard/opentom/fonts/times.ttf", 11);
    } 
    
    if ( ! nb_file_in_list) {
        for(i=optind; i < argc; i++) {
            nb_file_in_list++;
            file_list[nb_file_in_list] = argv[i];
        }
        printf("loaded %d images\n", nb_file_in_list);
    }

    screen = initScreen();
    picturePortion.w = SCREENWIDTH;
    picturePortion.h = SCREENHEIGHT;

    image = loadImage(&currentImageNumber);
    if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
        scaledImage = zoom100(image, &picturePortion, &scale);
    } else {
        scaledImage = zoomFit(image, &picturePortion, &scale);
    }
    name = drawFileName(file_list[currentImageNumber], font, runSlideShow);
    drawImage(scaledImage, &picturePortion, screen, name);

    int ox, oy, cmd; Uint32 time, dbClick = 0, last_up = 0;
    do {
        SDL_Event event;
        if ( SDL_WaitEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    event.type = SDL_KEYDOWN;
                    event.key.keysym.sym = SDLK_q;
                    break;
                    
                case SDL_VIDEORESIZE:
                    printf("resize[%d](%d,%d)\n", event.resize.type, event.resize.w, event.resize.h);
                    scrWidth = event.resize.w;
                    scrHeight = event.resize.h;
                    picturePortion.w = SCREENWIDTH;
                    picturePortion.h = SCREENHEIGHT;
                    SDL_SetVideoMode(picturePortion.w = scrWidth = event.resize.w, 
                                        picturePortion.h =scrHeight = event.resize.h, SCREENBPP,
                                           SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);
                    setTitle();
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                        if (SDL_GetTicks()-last_up < 250) {
                            dbClick=1;
                            printf("dbClick\n");
                        }
                        ox = event.button.x;
                        oy = event.button.y;
                        time = SDL_GetTicks (); 
                        continue;
                        
                case SDL_MOUSEBUTTONUP:
                        printf("delta=%d", SDL_GetTicks()-time);
                    if ( (last_up =SDL_GetTicks())-time < 250) {   
                        cmd = (event.button.x / (screen->w/3))*10 + event.button.y / (screen->h/3);                        
                        switch(cmd) {
                            case 0: // rotate left
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_l;
                                break;
                            case 10: // zoumOut
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_o;
                                break;
                            case 20: // rotate right
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_r;
                                break;
                            case 1: // prev picture
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_p;
                                break;
                            case 11: 
                                event.type = SDL_KEYDOWN;
                                if ( dbClick ) { // toggle full screen
                                    screen = toggleFullScreen();
                                    scrWidth = picturePortion.w=screen->w;
                                    scrHeight = picturePortion.h=screen->h;
                                }// else // feet to screen
                                event.key.keysym.sym = SDLK_f;
                                break;
                            case 21: // next picture
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_n;
                                break;     
                            case 2:// zoom 100%
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_z;
                                break; 
                            case 12: // zoumIn
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_i;
                                break;
                            case 22: // start/stop slide show
                                event.type = SDL_KEYDOWN;
                                event.key.keysym.sym = SDLK_s;
                                break;
                        }
                        time = dbClick= 0;
                        break;
                    }
                    time = dbClick= 0;
                    continue;
                    
                case SDL_MOUSEMOTION:
                    if ( event.button.button && ((event.button.x-ox) || (event.button.y-oy))) {
                        pan(scaledImage, &picturePortion, ox-event.button.x, oy-event.button.y);
                        ox = event.button.x;
                        oy = event.button.y;
                        drawImage(scaledImage, &picturePortion, screen, showFileName ? name : 0);
                    } 
                    continue;
                        
                default: 
                    printf("%d %d\n", event.type, SDL_KEYDOWN);
            }
    
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT: // PAN LEFT
                    pan(scaledImage, &picturePortion, -PANSTEP, 0);
	                break;
                case SDLK_RIGHT: // PAN RIGHT
                    pan(scaledImage, &picturePortion, PANSTEP, 0);
                    break;
                case SDLK_UP: // PAN UP
                    pan(scaledImage, &picturePortion, 0, -PANSTEP);
                    break;
                case SDLK_DOWN: // PAN DOWN
                    pan(scaledImage, &picturePortion, 0, PANSTEP);
                    break;
                case SDLK_i: // ZOOM IN
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomIn(image, &picturePortion, &scale);
                    break;
                case SDLK_o: // ZOOM OUT
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomOut(image, &picturePortion, &scale);
                    break;
                case SDLK_f: // ZOOM TO FIT SCREEN
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomFit(image, &picturePortion, &scale);
                    break;
                case SDLK_z: // ZOOM TO ORIGINAL SIZE
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoom100(image, &picturePortion, &scale);
                    break;
                case SDLK_l: // ROTATE LEFT
                    {
                        SDL_FreeSurface(scaledImage);
                        SDL_Surface *tmp = (SDL_Surface *)rotateSurface90Degrees(image, 3);
                        SDL_FreeSurface(image);
                        image = tmp;
                        scaledImage = (SDL_Surface *)zoomSurface(image, scale, scale, SMOOTHING_ON);
                        int x = picturePortion.x;
                        picturePortion.x = picturePortion.y + SCREENHEIGHT/2 - SCREENWIDTH/2;
                        picturePortion.y = scaledImage->h - x - SCREENHEIGHT/2 - SCREENWIDTH/2;
                        pan(scaledImage, &picturePortion, 0, 0);
                    }
                    break;
                case SDLK_r: // ROTATE RIGHT
                    {
                        SDL_FreeSurface(scaledImage);
                        SDL_Surface *tmp = (SDL_Surface *)rotateSurface90Degrees(image, 1);
                        SDL_FreeSurface(image);
                        image = tmp;
                        scaledImage = (SDL_Surface *)zoomSurface(image, scale, scale, SMOOTHING_ON);
                        int x = picturePortion.x;
                        picturePortion.x = scaledImage->w - picturePortion.y - SCREENWIDTH/2
                                           - SCREENHEIGHT/2;
                        picturePortion.y = x + SCREENWIDTH/2 - SCREENHEIGHT/2;
                        pan(scaledImage, &picturePortion, 0, 0);
                    }
                    break;
                case SDLK_n: // NEXT IMAGE
                    if (currentImageNumber < argc-1) ++currentImageNumber;
                    else currentImageNumber = 1;
                    
                    {
                        SDL_FreeSurface(image);
                        SDL_FreeSurface(scaledImage);
                        SDL_FreeSurface(name);

                        image = loadImage(&currentImageNumber);
                        if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
                            scaledImage = zoom100(image, &picturePortion, &scale);
                        } else {
                            scaledImage = zoomFit(image, &picturePortion, &scale);
                        }
                        name = drawFileName(file_list[currentImageNumber], font, runSlideShow);
                    }/* else {
                        if (runSlideShow) {
                            SDL_RemoveTimer(slideShowTimer);
                            runSlideShow = FALSE;
                            name = drawFileName(argv[currentImageNumber], font, runSlideShow);
                        }
                    }*/
                    break;
                case SDLK_p: // PREVIOUS IMAGE
                    if (currentImageNumber > 1) {
                        --currentImageNumber;

                        SDL_FreeSurface(image);
                        SDL_FreeSurface(scaledImage);
                        SDL_FreeSurface(name);

                        image = loadImage(&currentImageNumber);
                        if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
                            scaledImage = zoom100(image, &picturePortion, &scale);
                        } else {
                            scaledImage = zoomFit(image, &picturePortion, &scale);
                        }
                        name = drawFileName(file_list[currentImageNumber], font, runSlideShow);
                        setTitle();
                    }
                    break;
                case SDLK_s: // START / STOP SLIDESHOW
                    runSlideShow = 1 - runSlideShow;
                    name = drawFileName(file_list[currentImageNumber], font, runSlideShow);
                    if (runSlideShow) {
                        slideShowTimer = SDL_AddTimer(SLIDESHOWTIMEOUT, timerCallback, NULL);
                    } else {
                        SDL_RemoveTimer(slideShowTimer);
                    }
                    break;
                case SDLK_d: // SHOW / HIDE FILENAME
                    showFileName = 1 - showFileName;
                    break;
                case SDLK_ESCAPE: // QUIT
                case SDLK_q:
                    isRunning = FALSE;
                    break;
                default:
                    break;
             } // end of switch (event.key.keysym.sym)
        } // end of if(SDL_WaitEvent())
        drawImage(scaledImage, &picturePortion, screen, showFileName ? name : 0);
      }
    } while(isRunning); // end of do

    SDL_FreeSurface(image);
    SDL_FreeSurface(scaledImage);
    SDL_FreeSurface(screen);

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_Quit();

    return 0;
}

