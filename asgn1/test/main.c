//#include <plnprint.h>
#include "stdio.h"
#include "../malloc.h"

#define PRINT_BUF_SIZE (1500)


int main(int argc, char** argv) {
//    printf("size of heapinfo %ld, size of heapchunk %ld\n", 
//            sizeof(struct HeapInfo_t), sizeof(struct HeapChunk_t));
    char* p1 = (char*) mymalloc(PRINT_BUF_SIZE);
    char* p2 = NULL;
    //char* p2 = (char*) mymalloc(PRINT_BUF_SIZE);
    fprintf(stderr, "p1 at %p p2 at %p\n", p1, p2);

    print_heap();

 //   plnprintf(stdout, "testing print");

  return 0;
}
