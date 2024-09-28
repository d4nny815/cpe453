#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include <unistd.h>
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#define HEAP_INC_AMOUNT (1 << 16)
#define VOIDPTR_TO_INTPTR(x) ((intptr_t*)x)
#define CHUNK_HEADER_SIZE (sizeof(struct HeapChunk_t))
#define IS_DIV_16(x) (!(x % 16))


struct HeapInfo_t {
  struct HeapChunk_t* start_ptr;
  size_t avail_mem;
};

struct HeapChunk_t {
  struct HeapChunk_t* next;
  struct heapChunk_t* prev;
  bool in_use;
  int size;
};



void* mymalloc(size_t size);
void* myrealloc(void* ptr, size_t size);
void* mycalloc(size_t nmemb, size_t size);
void myfree(void* ptr);


void init_heap();
void print_heap();
void print_chunk(struct HeapChunk_t* chunk);
struct HeapChunk_t* create_chunk(size_t size);
void* get_chunk_data_ptr(struct HeapChunk_t* chunk);
struct HeapChunk_t* get_next_free_chunk(size_t size);

#endif /* LIBMALLOC_H */
