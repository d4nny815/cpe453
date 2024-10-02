#include "stdio.h"
#include "malloc.h"

#define PRINT_BUF_SIZE (1500)
static char buf[PRINT_BUF_SIZE];

// int main(int argc, char** argv) {
int main(void) {
    print_heap();
    char *p1, *p2, *p3, *p4;
    int i;
    for (i = 0; i < 8192; i++) {
        size_t size = i;
        p1 = malloc(size);
        if ((long)p1 & (long)0xf) {
            return 1;
        }
        snprintf(buf, PRINT_BUF_SIZE, "%p", p1);
        puts(buf);
    }
    
    print_heap();
    /*
    //p1 = (char*) malloc(PRINT_BUF_SIZE);
    p1 = malloc(1024);
    p2 = (char*) malloc(2 * PRINT_BUF_SIZE);
    p3 = (char*) malloc(PRINT_BUF_SIZE / 2);
    p4 = (char*) malloc(PRINT_BUF_SIZE / 10);
    snprintf(buf, PRINT_BUF_SIZE, "p1: %p, p2: %p, p3: %p, p4:%p", 
            p1, p2, p3, p4);
    puts(buf);
    print_heap();

    free(p1);
    free(p3);
    fprintf(stderr, "free p1 and p3\n");
    print_heap();

    p1 = malloc(PRINT_BUF_SIZE / 2);
    p3 = malloc(PRINT_BUF_SIZE * 2);
    fprintf(stderr, "p1: %p, p2: %p, p3: %p, p4:%p\n", p1, p2, p3, p4);
    print_heap();

    free(p2);
    fprintf(stderr, "free p2\n");
    print_heap();

    free(p4);
    fprintf(stderr, "free p4\n");
    print_heap();
    */
  return 0;
}

