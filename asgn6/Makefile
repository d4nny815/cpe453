CC = gcc
CFLAGS = -Wall -pedantic -ansi

.PHONY: all test clean

all: hello

test: hello
	@./$<

hello: hello.o
	$(CC) $(CFLAGS) $< -o $@

hello.o: hello.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f hello.o hello
