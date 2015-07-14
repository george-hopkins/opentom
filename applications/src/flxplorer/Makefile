NAME=flxplorer
EXT_FLAGS=-I$(ROOT)/kernel/include -I$(ARM_SYSROOT)/usr/include/microwin

install: $(NAME)
	cp $(NAME) $(TOMDIST)/bin

$(NAME): $(NAME).cxx
	$(CXX) -o $@ $(EXT_FLAGS) $(NAME).cxx -L$(ARM_SYSROOT)/usr/lib -lfltk
	$(STRIP) $@

clean:
	rm -f $(NAME) *.o
