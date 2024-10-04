#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define HEAP_INC_STEP       (1 << 16)
#define INIT_HEAP_PASSED    (0)
#define HEAP_NOMEM_AVAIL    (1)
#define CHUNK_HEADER_SIZE   (sizeof(struct HeapChunk_t))
#define MIN_CHUNK_DATA      (16)
#define MIN_CHUNK_SPACE     ((intptr_t)(MIN_CHUNK_DATA + CHUNK_HEADER_SIZE))
#define ENOUGH_MEM          (0)
#define GET_DIV16_VAL(x)    ((x + 15) & ~0xf)


struct HeapInfo_t {
  struct HeapChunk_t* p_start;
  struct HeapChunk_t* p_last;
  intptr_t end_addr;
  bool exists;
};


typedef struct HeapChunk_t {
  struct HeapChunk_t* next;
  struct HeapChunk_t* prev;
  size_t size;
  bool in_use;
} HeapChunk_t;


void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);




#endif /* LIBMALLOC_H */
