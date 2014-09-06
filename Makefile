
include .make_config

export PKG_CONFIG_PATH = $(PREFIX)/lib/pkgconfig:/usr/lib64/pkgconfig 
export LIBS_DEPS = libdomhp

CC = gcc -std=gnu99

BINS = lingvo-server

lingvo_server_OBJECTS = lingvo-server.o

LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(LIBS_DEPS))
 
INCLUDE = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(LIBS_DEPS))
 
all: $(BINS)

lingvo-server: $(lingvo_server_OBJECTS)
	$(CC) -o lingvo-server $(lingvo_server_OBJECTS) $(LIBS)

$(lingvo_server_OBJECTS): %.o : %.c
	$(CC) $(INCLUDE) -c $*.c

install:

clean:
	rm -vf $(BINS) $(lingvo_server_OBJECTS)

