#include "malloc.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    void *val;
    val = malloc (1024);
    if ( val ) {
        printf("Calling malloc succeeded.\n");
        exit(0);
    }
    return 1;
}


