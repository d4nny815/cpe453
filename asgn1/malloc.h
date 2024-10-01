#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define HEAP_INC_STEP     (1 << 16)
//#define HEAP_INC_STEP       (1 << 10)
#define CHUNK_HEADER_SIZE   (sizeof(struct HeapChunk_t))
#define INIT_HEAP_PASSED    (0)
#define HEAP_NOMEM_AVAIL    (1)


struct HeapInfo_t {
  struct HeapChunk_t* p_start;
  size_t avail_mem;
  bool exists;
};


typedef struct HeapChunk_t {
  struct HeapChunk_t* next;
  struct HeapChunk_t* prev;
  bool in_use;
  size_t size;
} HeapChunk_t;


void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);

int init_heap();

HeapChunk_t* create_chunk(size_t size);
void* get_chunk_data_ptr(HeapChunk_t* chunk);
HeapChunk_t* get_free_chunk(size_t size);
HeapChunk_t* get_pchunk_from_pdata(void* ptr);

size_t make_div_16(size_t val);

void print_heap();
void print_chunk(HeapChunk_t* chunk);


#endif /* LIBMALLOC_H */
