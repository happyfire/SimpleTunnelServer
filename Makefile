CC = gcc

CFLAGS :=-g -Wall
CPPFLAGS += -I/usr/include/libev
LDFLAGS += -lev


SOURCES = src/tunnel_server.c src/base/cache.c src/base/utils.c
OBJECTS = $(SOURCES:.c=.o)

all: server

server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	@echo "Cleanning project"
	-rm *.o server
	-rm src/*.d src/base/*.d
	@echo "Clean complete"

.PHONY:clean

include $(SOURCES:.c=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$