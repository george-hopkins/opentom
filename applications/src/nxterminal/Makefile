install: nxterminal
	cp nxterminal $(TOMDIST)/bin

nxterminal: actions.cpp gterm.cpp nxterminal.cxx states.cpp utils.cpp vt52_states.cpp
	$(CXX) -g -o nxterminal *.c*  -lnano-X
	
clean:
	rm -f nxterminal *.o
