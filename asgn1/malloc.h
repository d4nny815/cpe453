#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

// #define HEAP_INC_AMOUNT     (1 << 16)
#define HEAP_INC_STEP       (1 << 10)
#define MASK_16             (~0xf)
#define CHUNK_HEADER_SIZE   (sizeof(struct HeapChunk_t))
#define IS_DIV_16(x)        (!(x % 16))
#define MAKE_DIV_16(x)      ((x + 16) & MASK_16)
#define INIT_HEAP_PASSED    (0)


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


void* mymalloc(size_t size);
void* myrealloc(void* ptr, size_t size);
void* mycalloc(size_t nmemb, size_t size);
void myfree(void* ptr);

int init_heap();

HeapChunk_t* create_chunk(size_t size);
void* get_chunk_data_ptr(HeapChunk_t* chunk);
HeapChunk_t* get_free_chunk(size_t size);


size_t make_div_16(size_t val);

void print_heap();
void print_chunk(HeapChunk_t* chunk);


#endif /* LIBMALLOC_H */
