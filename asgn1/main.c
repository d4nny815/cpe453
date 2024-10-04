#include "stdio.h"
#include "malloc.h"

#define PRINT_BUF_SIZE (1500)
static char buf[PRINT_BUF_SIZE];

// int main(int argc, char** argv) {
int main(void) {
    char *p1, *p2, *p3, *p4;
    int i;
    for (i = 0; i < 128; i++) {
        size_t size = i;
        p1 = malloc(size);
        if ((long)p1 & (long)0xf) {
            return 1;
        }
        snprintf(buf, PRINT_BUF_SIZE, "%p with size %zu", p1, size);
        puts(buf);
    }
    
    p1 = (char*) malloc(PRINT_BUF_SIZE);
    //p1 = malloc(1024);
    p2 = (char*) malloc(2 * PRINT_BUF_SIZE);
    p3 = (char*) malloc(PRINT_BUF_SIZE / 2);
    p4 = (char*) malloc(PRINT_BUF_SIZE / 10);
    snprintf(buf, PRINT_BUF_SIZE, "p1: %p, p2: %p, p3: %p, p4:%p\n", 
            p1, p2, p3, p4);
    write(STDERR_FILENO, buf, strlen(buf));
    free(p1);
    free(p3);
    fprintf(stderr, "free p1 and p3\n");

    p1 = malloc(PRINT_BUF_SIZE / 2);
    p3 = malloc(PRINT_BUF_SIZE * 2);
    fprintf(stderr, "p1: %p, p2: %p, p3: %p, p4:%p\n", p1, p2, p3, p4);

    free(p2);
    fprintf(stderr, "free p2\n");

    free(p4);
    fprintf(stderr, "free p4\n");
  return 0;
}

