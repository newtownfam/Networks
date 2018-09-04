# Project 1 Makefile
# Peter Christakos

CC = gcc
CFLAGS = -Wall
OBJS = http_client.o

all: http_client

http_client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o http_client

http_client.o: http_client.c 
	$(CC) $(CFLAGS) -c http_client.c

clean:
	rm -f *~ *.o http_client
