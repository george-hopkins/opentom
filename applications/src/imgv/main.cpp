#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL_image.h>

SDL_Surface *screen;

int display_image( char *image_name) {
    
    SDL_Surface *screen, *image=IMG_Load(image_name);
    SDL_Rect rect;
    
    if (image==NULL)
    {
        fprintf(stderr,"Couldn't load image\n");
        return 0;
    }
    
    if(SDL_SetColorKey(image,SDL_RLEACCEL,SDL_MapRGB(image->format,0,0,0)) != 0)
    {
        fprintf(stderr,"Couldn't ser color key: %s\n",SDL_GetError());
	return 0;
    }
    
    image=SDL_DisplayFormat(image);
    rect.x=100;rect.y=100;rect.w=image->w;rect.h=image->h;
    SDL_BlitSurface(image,0,screen,&rect);
    SDL_UpdateRects(screen,1,&rect);
    
}

int main(int argc, char** argv) {

    SDL_Rect **r;
    if ( SDL_Init( SDL_INIT_VIDEO) == -1 ) {
        fprintf(stderr, "Error: Couldn't initialise SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    r= SDL_ListModes(NULL, SDL_FULLSCREEN);
    screen = SDL_SetVideoMode( (*r)[0].w, (*r)[0].h, 0, SDL_SWSURFACE | SDL_FULLSCREEN);
    if(screen==NULL) {
        fprintf(stderr,"Couldn't set video mode: %s\n", SDL_GetError());
	return 1;
    }
    SDL_SetClipRect(screen, NULL);
    display_image( "splash.bmp");
    
    SDL_Event event;
    int exited = 0;
    while(exited==0)
    {
        if(SDL_PollEvent(&event)==1)
        {
            if((event.type==SDL_MOUSEBUTTONUP)) exited=1;
            if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT))
                exited=1;
        }
    }
    return 0;
}

