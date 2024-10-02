#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define HEAP_INC_STEP       (1 << 16)
#define INIT_HEAP_PASSED    (0)
#define HEAP_NOMEM_AVAIL    (1)
#define CHUNK_HEADER_SIZE   (sizeof(struct HeapChunk_t))
#define MIN_CHUNK_DATA      (16)
#define MIN_CHUNK_SPACE     (MIN_CHUNK_DATA + CHUNK_HEADER_SIZE)
#define GET_DIV16_ADDR(x)   ((x + 15) & ~0xf)


struct HeapInfo_t {
  struct HeapChunk_t* p_start;
  size_t avail_mem;
  bool exists;
};


typedef struct HeapChunk_t {
  bool in_use;
  struct HeapChunk_t* next;
  struct HeapChunk_t* prev;
  size_t size;
} HeapChunk_t;


void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);

static int init_heap();

static HeapChunk_t* get_free_chunk(size_t size);
static void split_chunk(HeapChunk_t* chunk, size_t size);

static HeapChunk_t* get_pchunk_from_pdata(void* ptr);
static void* get_chunk_data_ptr(HeapChunk_t* chunk);

static void print_heap();
static void print_chunk(HeapChunk_t* chunk);


#endif /* LIBMALLOC_H */
