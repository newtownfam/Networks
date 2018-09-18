# CS Networks Project 2 Makefile
# Peter Christakos
# pechristakos@wpi.edu

CC = gcc
CFLAGS = -Wall -g
OBJS = project2.o project2.h

all: project2

project2: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o project2

project2.o: project2.c
	$(CC) $(CFLAGS) -c project2.c student2.c

clean:
	rm -f *~ *.o project2