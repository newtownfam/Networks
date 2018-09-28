# CS Networks Project 2 Makefile
# Peter Christakos
# pechristakos@wpi.edu

CC = gcc
CFLAGS = -g
OBJS = project2.o student2.o

all: project2

project2: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o project2

project2.o: project2.c
	$(CC) $(CFLAGS) -c project2.c student2.c

clean:
	rm -f *~ *.o project2 student2