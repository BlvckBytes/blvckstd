CC        := g++
CFLAGS    := -Wall

CPPFLAGS  += -I../include
CPPFLAGS  += -lblvckstd

all: jsonh_getters jsonh_parse jsonh_stringify

jsonh_getters:
	$(CC) $(CPPFLAGS) $(CFLAGS) jsonh_getters.cpp -o jsonh_getters.out

jsonh_parse:
	$(CC) $(CPPFLAGS) $(CFLAGS) jsonh_parse.cpp -o jsonh_parse.out

jsonh_stringify:
	$(CC) $(CPPFLAGS) $(CFLAGS) jsonh_stringify.cpp -o jsonh_stringify.out

clean:
	rm -rf *.out
