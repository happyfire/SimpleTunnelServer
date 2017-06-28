CC = gcc

CFLAGS=-g -Wall
LDFLAGS += -lev

all: server

server: tunnel_server.o
	$(CC) $(LDFLAGS) -o server tunnel_server.o

clean:
	-rm *.o server
