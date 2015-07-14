#nanox/keyboard/Makefile

# Note:  We only install for one keyboard to cut down
# on space usage.  It is unfortunately that we have to
# use .bmp files, but thats the way the cookie crumbles

KEYMAP_PREFIX=com

TARGET=nxkeyboard

OBJS=cli.o filetools.o keymap.o nxkeyboard.o srvconn.o
INSTALL_EXTRAS=inst-keymaps

LIBS+=-lwm -lnano-X

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

include $(BASE_DIR)/Rules.make

inst-keymaps:
	mkdir -p $(INSTALL_DIR)/share/keymaps
	cp -af keymaps/$(KEYMAP_PREFIX)* $(INSTALL_DIR)/share/keymaps	
