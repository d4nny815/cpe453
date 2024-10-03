C = gcc
CFLAGS = -Wall -g -fPIC

.PHONY: malloc clean core_clean run test

all: intel-all malloc main

# BUILD LIBS
malloc: libmalloc.so libmalloc.a

libmalloc.a: malloc.o
	ar rcs libmalloc.a malloc.o

libmalloc.so: malloc.o
	$(CC) $(CFLAGS) -shared -o libmalloc.so malloc.o

malloc.o: malloc.c intel-all
	$(CC) $(CFLAGS) -c malloc.c


# NICO GIVEN	
intel-all: lib/libmalloc.so lib64/libmalloc.so

lib/libmalloc.so: lib malloc32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o
	$(CC) $(CFLAGS) -shared -o $@ malloc64.o
						    
lib:
	mkdir lib

lib64:
	mkdir lib64

malloc32.o: malloc.c
	$(CC) $(CFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c -o malloc64.o malloc.c


# MY STUFF
main: main.o malloc
	$(CC) -g -L./lib64 -o main main.o -lmalloc
	
main.o: main.c
	$(CC) -Wall -g -c -o main.o main.c

run: main
	./main

gdb: main
	gdb ./main


nico_test: malloc 
	$(CC) -g -L./lib64 -o nico_test lib.c nico_test.c -lmalloc
	./nico_test


clean:
	rm -rf *.o *.a *.so main lib lib64 nico_test.o

core_clean:
	rm -rf core.*

test:
	~pn-cs453/demos/tryAsgn1 > test.log
