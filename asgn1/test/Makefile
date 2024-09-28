CC = gcc
CFLAGS = -Wall -g 
NICO_LIB = ~pn-cs453/demos/lib 

.PHONY: run clean


main: main.o
	$(CC) -L $(NICO_LIB) -o main main.o -lmalloc
#	$(CC) -o hello -I ~pn-cs453/Given/Asgn1 -L ~pn-cs453/Given/Asgn1/lib64 main.c -lplnprint
main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

run: main
	./main

clean:
	rm -f main.o 
