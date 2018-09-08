# Project 1 Makefile
# Peter Christakos

CC = gcc
CFLAGS = -Wall
OBJS = http_client.o
OBJS2 = http_server.o 

all: http_client http_server

http_client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o http_client

http_client.o: http_client.c 
	$(CC) $(CFLAGS) -c http_client.c

http_server: $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o http_server

http_server.o: http_server.c 
	$(CC) $(CFLAGS) -c http_server.c

clean:
	rm -f *~ *.o http_client http_server
