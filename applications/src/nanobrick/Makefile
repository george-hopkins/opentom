install: nanobrick nanobrick_editor
	mkdir -p $(TOMDIST)/share/nanobrick_levels
	cp nanobrick nanobrick_editor $(TOMDIST)/bin

nanobrick: nanobrick_images.c nanobrick.c
	$(CC) -o nanobrick nanobrick.c -I$(ARM_APPROOT)/include/microwin -lnano-X

nanobrick_editor: nanobrick_images.c nanobrick_editor.c
	$(CC) -o nanobrick_editor nanobrick_editor.c -I$(ARM_APPROOT)/include/microwin -lnano-X

nanobrick_images.c: nanobrick_images.bmp
	convbmp nanobrick_images.bmp

clean:
	rm -f nanobrick_images.c nanobrick nanobrick_editor *.o

