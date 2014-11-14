
include .make_config

export PKG_CONFIG_PATH = $(PREFIX)/lib/pkgconfig:/usr/lib64/pkgconfig 
export LIBS_DEPS = libdomhp lingvo

CC = gcc -std=gnu99

BINS = lingvo-server test

lingvo_server_OBJECTS = \
	core.o \
	doc-template.o \
	handler-default.o \
	handler-dictionary.o \
	handler-err.o \
	handler-file.o \
	handler-shutdown.o \
	handler-test.o \
	lingvo-server.o \
	lingvo-server-request.o \
	lingvo-server-request-handler.o \
	lingvo-server-utils.o \
	multipart-data.o \
	query-string.o

test_OBJECTS = \
	doc-template.o \
	lingvo-server-utils.o \
	test.o

DEFINES = -DLINGVO_FILES_DIR=\"$(LINGVO_FILES_DIR)\"

OBJECTS = $(test_OBJECTS) $(filter-out $(test_OBJECTS),$(lingvo_server_OBJECTS))

LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(LIBS_DEPS))
 
INCLUDE = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(LIBS_DEPS))
 
 
all: $(BINS)

$(OBJECTS): %.o : %.c
	$(CC) $(DEFINES) $(INCLUDE) -c -o $@ $*.c
ifeq ($(is_preproc),yes)
	$(CC) -E $(DEFINES) $(INCLUDE) -c -o $*.i $*.c
endif

lingvo-server: $(lingvo_server_OBJECTS)
	$(CC) -o lingvo-server $(lingvo_server_OBJECTS) $(LIBS)

test: $(test_OBJECTS)
	$(CC) -o test $(test_OBJECTS) $(LIBS)

install:

clean:
	rm -vf $(BINS) $(OBJECTS)

