CC = gcc

CFLAGS :=-g -Wall -Werror -std=gnu99
CPPFLAGS += -I/usr/include/libev
LDFLAGS += -lev

SOURCES = src/main.c src/tunnel_server.c src/client.c src/utils.c \
          src/server_ip.c src/server_connect.c src/server_transout.c src/server_transin.c
OBJECTS = $(SOURCES:.c=.o)

all: server

server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	@echo "Cleanning project"
	-rm src/*.o src/*.d server
	@echo "Clean complete"

.PHONY:clean

include $(SOURCES:.c=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$