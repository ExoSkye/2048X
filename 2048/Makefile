XBE_TITLE = 2048
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.cpp
NXDK_DIR = $(CURDIR)/../..
NXDK_SDL = y
NXDK_CXX = y
all_local: cp_img all

include $(NXDK_DIR)/Makefile

cp_img:
	@mkdir -p $(OUTPUT_DIR)
	cp *.png $(OUTPUT_DIR)/
