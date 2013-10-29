CC = gcc
CFLAGS = -std=c99 -fgnu89-inline -Wall -O0 -g $(GLIB_CFLAGS)
LIBS = $(GLIB_LIBS)

GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS := $(shell pkg-config --libs glib-2.0)

TARGETS = botserv

botserv_SOURCES = botserv.c conf.c sock.c irc_msg.c irc.c bots.c channel.c irc_types.c server.c user.c
botserv_OBJECTS = $(botserv_SOURCES:.c=.o)
botserv_DEPENDS = $(botserv_SOURCES:.c=.P)

all: $(TARGETS)

botserv: $(botserv_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -MD -c $< -o $@
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(depends)

.PHONY: clean
clean:
	-rm $(TARGETS)
	-rm $(botserv_OBJECTS) $(botserv_DEPENDS)

