
install: $(CROSS)/bin/get_source.sh
	# Env Sanity checks
	@if [ -z $(ROOT) ]; then echo "Please, do first : source get_build_env.sh"; false; fi
	@if [ "$(ROOT)" != "$(PWD)" ]; then echo "The ROOT environment variable $(ROOT) != $(PWD) doesn't match the current directory"; fi

	@if test -d /usr/local/cross; then \
		ls -l /usr/local/cross | grep $(ROOT) || { \
			echo "WRONG LINK: /usr/local/cross !=> $(ROOT)"; false; } \
	else \
		echo Must be root to make /usr/local/cross link :; \
		sudo ln -s $(ROOT) /usr/local/cross; \
	fi
	make $(CROSS)/bin/get_source.sh


$(CROSS)/bin/get_source.sh:
	mkdir -p $(CROSS)/bin
	cp get_source.sh install_shared_libs.sh is_use_lib.sh $(CROSS)/bin
	make -C ttimage_tools install

clean:
	make -C ttimage_tools clean

