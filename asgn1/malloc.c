#include "malloc.h"
#include <unistd.h>

// static struct HeapInfo_t heap_info = {NULL, 0};

void init_heap() {
  void* start_ptr = sbrk(0);
  // heap_info.start_ptr = (struct heapchunk_t*)start_ptr;
  
  printf("heap starts at %p\n", start_ptr);   
  
  return;
}


/*
    check if heap exist
    if no
        init heap
    


*/
// void* mymalloc(size_t size) {


// }