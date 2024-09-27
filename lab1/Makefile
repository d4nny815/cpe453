PROJECT = pipeit

CC = gcc
CFLAGS = -Wall -Werror -std=c99 -g

all: $(PROJECT)

$(PROJECT).o: $(PROJECT).c 
	$(CC) $(CFLAGS) -o $@ -c $<


run: $(PROJECT) # only stdout
	./$(PROJECT) 2>/dev/null

debug: $(PROJECT) # stdout and stderr
	./$(PROJECT)

error: $(PROJECT) # only stderr
	./$(PROJECT) >/dev/null

clean:
	rm -f $(PROJECT) $(PROJECT).o outfile

