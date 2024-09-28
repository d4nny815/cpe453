CC = gcc
CFLAGS = -Wall -g 
NICO_LIB = ~pn-cs453/demos/lib 

.PHONY: run clean


main: main.o ../malloc.o
#	$(CC) -L $(NICO_LIB) -o main main.o -lmalloc
#	$(CC) -o hello -I ~pn-cs453/Given/Asgn1 -L ~pn-cs453/Given/Asgn1/lib64 main.c -lplnprint
	$(CC) $(CFLAGS) main.o ../malloc.o -o main

main.o: main.c 
	$(CC) $(CFLAGS) -c main.c -o main.o 

../malloc.o: ../malloc.h ../malloc.c
	$(CC) $(CFLAGS) -c ../malloc.c -o ../malloc.o

run: main
	clear
	./main

clean:
	rm -f main main.o ../malloc.o 
