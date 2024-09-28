#include "stdio.h"
#include "libmalloc.h"

int main(int argc, char** argv) {
  printf("size of heapinfo %ld, size of heapchunk %ld\n", sizeof(struct HeapInfo_t), sizeof(struct Heapchunk_t));

  init_heap();

  return 0;
}