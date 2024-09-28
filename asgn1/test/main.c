//#include <plnprint.h>
#include "stdio.h"


#define PRINT_BUF_SIZE (1500)


int main(int argc, char** argv) {
    // printf("size of heapinfo %ld, size of heapchunk %ld\n", sizeof(struct HeapInfo_t), sizeof(struct Heapchunk_t));

    char print_buf[PRINT_BUF_SIZE];
    snprintf(print_buf, PRINT_BUF_SIZE, "hello world\n");
    puts(print_buf);

 //   plnprintf(stdout, "testing print");

  // init_heap();

  return 0;
}
