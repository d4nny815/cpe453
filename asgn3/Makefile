CC = gcc
CFLAGS = -Wall -Werror -g

.PHONY = all clean

all: dine

dine: dine.o 
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lrt

dine.o: dine.c dine.h
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -rf *.o dine