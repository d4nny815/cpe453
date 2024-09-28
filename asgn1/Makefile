CC = gcc
CFLAGS = -Wall -std=c99 -g

.PHONY: run clean


main: main.o
	$(CC) $(CFLAGS) main.o libmalloc.o -o main

main.o: main.c
	$(CC) -L nico_lib $(CFLAGS) main.c -o main.o


intel-all: lib/libmalloc.so lib64/libmalloc.so

lib/libmalloc.so: lib malloc32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o
	$(CC) $(CFLAGS) -shared -o $@ malloc64.o

lib: 
	mkdir lib

lib64:
	mkdir lib64

malloc32.o: malloc.
	$(CC) $(CFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c -o malloc64.o malloc.c

run: main
	./main

clean:
	rm -f *~ TAGS
