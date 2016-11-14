install:
	# Environment sanity checks
	@if [ -z $(ROOT) ]; then echo "Please, do first : source get_build_env.sh"; false; fi
	@if [ "$(ROOT)" != "$(PWD)" ]; then echo "The ROOT environment variable $(ROOT) != $(PWD) doesn't match the current directory"; fi
	install -d $(ROOT)/tools
	install -t $(ROOT)/tools get_source.sh install_shared_libs.sh is_use_lib.sh
	make -C ttimage_tools install

clean:
	make -C ttimage_tools clean
