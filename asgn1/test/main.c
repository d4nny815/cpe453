//#include <plnprint.h>
#include "stdio.h"
#include "../malloc.h"

#define PRINT_BUF_SIZE (1500)


// int main(int argc, char** argv) {
int main(void) {
    print_heap();

    char* p1 = (char*) mymalloc(PRINT_BUF_SIZE);
    char* p2 = (char*) mymalloc(2 * PRINT_BUF_SIZE);
    char* p3 = (char*) mymalloc(PRINT_BUF_SIZE / 2);
    char* p4 = (char*) mymalloc(PRINT_BUF_SIZE / 10);
    fprintf(stderr, "p1: %p, p2: %p, p3: %p, p4:%p\n", p1, p2, p3, p4);
    print_heap();

    myfree(p1);
    myfree(p3);
    fprintf(stderr, "free p1 and p3\n");
    print_heap();

    p1 = mymalloc(PRINT_BUF_SIZE / 2);
    p3 = mymalloc(PRINT_BUF_SIZE * 2);
    fprintf(stderr, "p1: %p, p2: %p, p3: %p, p4:%p\n", p1, p2, p3, p4);
    print_heap();

    myfree(p2);
    fprintf(stderr, "free p2\n");
    print_heap();

    myfree(p4);
    fprintf(stderr, "free p4\n");
    print_heap();

  return 0;
}
