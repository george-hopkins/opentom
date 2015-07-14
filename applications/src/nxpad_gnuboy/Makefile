NAME=nxpad_gnuboy

$(NAME): gamepad.cc
	$(CXX) $(CPPFLAGS) -o $@ gamepad.cc -L$(ARM_APPSYSROOT)/lib -lnano-X 

install: $(NAME)
	mkdir -p $(TOMDIST)/bin/$(NAME)
	cp $(NAME) nxpad2* $(TOMDIST)/bin/$(NAME)/
	echo echo '#! /bin/sh\n\ncd $$DIST/bin/$(NAME)\nexec ./$(NAME) nxpad2$1-skin.txt\n' >$(TOMDIST)/bin/$(NAME).sh; \

clean:
	rm -f $(NAME)
