CC = gcc
CFLAGS = -Wall -Werror -std=c99 -g

.PHONY: run clean


main: main.o libmalloc.o
	$(CC) $(CFLAGS) main.o libmalloc.o -o main

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

libmalloc.o: libmalloc.c libmalloc.h
	$(CC) $(CFLAGS) -c libmalloc.c -o libmalloc.o


run: main
	./main


clean:
	rm -f main main.o libmalloc.o

