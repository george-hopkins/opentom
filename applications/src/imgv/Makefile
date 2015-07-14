NAME=imgv

install: $(TOMDIST)/bin/$(NAME)

$(TOMDIST)/bin/$(NAME): $(NAME)
	cp $(NAME) $(TOMDIST)/bin

$(NAME): sdl-imageviewer.c  SDL_rotozoom.c
	$(CC) -o $@ $(EXT_FLAGS) *.c -lSDL -lSDL_image -lSDL_ttf

clean:
	rm -f $(NAME) *.o
