CC        := gcc
SRC_FILES := $(wildcard src/*.c) $(wildcard src/*/*.c)
TARG_LIB  := libblvckstd.dylib
CFLAGS    := -Wall -I./ -shared -o $(TARG_LIB)

$(TARG_LIB):
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC $(SRC_FILES)

clean:
	rm -rf $(TARG_LIB)
