#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include <unistd.h>

#define HEAP_AMOUNT (1 << 16)
#define VOIDPTR_TO_INTPTR(x) ((intptr_t*)x)

struct HeapInfo_t {
  struct heapchunk_t* start_ptr;
  size_t avail_mem;
};

struct Heapchunk_t {
  struct Heapchunk_t* next;
  struct heapchunk_t* prev;
  bool in_use;
  int size;
};


void* mymalloc(size_t size);
void* myrealloc(void* ptr, size_t size);
void* mycalloc(size_t nmemb, size_t size);
void myfree(void* ptr);


void init_heap();

#endif /* LIBMALLOC_H */