XBE_TITLE = 2048
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.cpp
NXDK_DIR = $(CURDIR)/../..
NXDK_SDL = y
NXDK_CXX = y
all_local: cp_img all
makeandmove: cp_img all

make_host:
	g++ main.cpp  -lSDL2 -I/usr/include/SDL2 -lSDL2_image -O0 -g

include $(NXDK_DIR)/Makefile

git_push:
	git add .
	git stage .
	git commit
	git push

cp_img:
	@mkdir -p $(OUTPUT_DIR)
	cp *.png $(OUTPUT_DIR)/
movetoshareddir:
	cp 2048.iso "/home/bob/shares/Shared Folder/"
