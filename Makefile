CC = gcc

CFLAGS :=-g -Wall
CPPFLAGS += -I/usr/include/libev
LDFLAGS += -lev


SOURCES = tunnel_server.c
OBJECTS = $(SOURCES:.c=.o)

all: server

server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	@echo "Cleanning project"
	-rm *.o server
	@echo "Clean complete"

.PHONY:clean

include $(SOURCES:.c=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$