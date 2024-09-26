#BIN=udpserver udpclient
CC=gcc
CFLAGS=-Wall -O0 -g
INCLUDE=-I .
LIBS=-lpthread
CFILES=$(wildcard ./*.c)
OBJS=$(patsubst %.c,%.o,$(CFILES))

all: server

server: server.o
	$(CC) $(CFLAGS) -o server server.o

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm server $(OBJS)

.PHONY: all clean
