#ifndef LIBMALLOC_H
#define LIBMALLOC_H

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#define HEAP_AMOUNT (1 << 16)


struct heapinfo_t {
    void* start_ptr;
    size_t avail_mem;
};

struct heapchunk_t {
    struct heapchunk_t* next;
    struct heapchunk_t* prev;
    bool in_use;
    int size;
};

extern struct heapinfo_t heap_info;


void* mymalloc(size_t size);
void* myrealloc(void* ptr, size_t size);
void* mycalloc(size_t nmemb, size_t size);
void myfree(void* ptr);


#endif /* LIBMALLOC_H */