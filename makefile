CC        := gcc
SRC_FILES := $(wildcard src/*.c) $(wildcard src/*/*.c)
CFLAGS    := -Wall -I./include -shared

TARG_LIB_PATH := /usr/local/lib
TARG_LIB  		:= libblvckstd.dylib

$(TARG_LIB):
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC $(SRC_FILES) -o $(TARG_LIB_PATH)/$(TARG_LIB)

clean:
	rm -rf $(TARG_LIB_PATH)/$(TARG_LIB)
