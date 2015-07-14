NAME=nxpad_gnuboy

$(NAME): gamepad.cc
	$(CXX) $(CPPFLAGS) -o $@ gamepad.cc -L$(ARM_APPSYSROOT)/lib -lnano-X 

install: $(NAME)
	cp $(NAME) $(TOMDIST)/bin
	mkdir -p $(TOMDIST)/share/$(NAME)
	cp $(NAME) nxpad2* $(TOMDIST)/share/$(NAME)/
	echo '#! /bin/sh§§cd $$DIST/share/$(NAME)§exec ./$(NAME) nxpad2$$*-skin.txt§' | tr '§' '\n' >$(TOMDIST)/bin/$(NAME).sh; \

clean:
	rm -f $(NAME)
