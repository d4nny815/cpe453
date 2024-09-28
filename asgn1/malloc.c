#include "malloc.h"

static struct HeapInfo_t heap_info;
static bool heap_exists = false;

void init_heap() {
    // TODO: error check sbrk
    void* start_ptr = sbrk(HEAP_INC_AMOUNT);
    heap_info.start_ptr = (struct HeapChunk_t*)start_ptr;
    heap_info.avail_mem = HEAP_INC_AMOUNT - CHUNK_HEADER_SIZE;  
        
    heap_info.start_ptr->next = NULL;
    heap_info.start_ptr->prev = NULL;
    heap_info.start_ptr->in_use = false;
    heap_info.start_ptr-> size = HEAP_INC_AMOUNT - CHUNK_HEADER_SIZE;

    heap_exists = true;
    //fprintf(stderr, "heap starts at %p with %ld free mem\n", 
    //        heap_info.start_ptr, heap_info.avail_mem);   
    //print_chunk(heap_info.start_ptr); 
    //fprintf(stderr, "data starts at %p\n", get_chunk_data_ptr(heap_info.start_ptr));
    return;
}


void* mymalloc(size_t size) {
    if (!heap_exists) {
        init_heap();
    }
    // size_t mem_to_give = size % heap_info.avail_mem;
    if (size > heap_info.avail_mem) {
        // TODO: ask for more mem
    }
    void* chunk_ptr = create_chunk(size);
    print_chunk(chunk_ptr);

    return get_chunk_data_ptr(chunk_ptr);
}

struct HeapChunk_t* create_chunk(size_t size) {
    struct HeapChunk_t* chunk_ptr = get_next_free_chunk(size);
    //fprintf(stderr, "free chunk at %p\n", chunk_ptr);

    struct HeapChunk_t* next_chunk = (struct HeapChunk_t*)
    ((((intptr_t)get_chunk_data_ptr(chunk_ptr)) + size + 16) & ~0xf);
    
    chunk_ptr->next = next_chunk;
    chunk_ptr->in_use = true;
    chunk_ptr->size = (size_t)((intptr_t)next_chunk - (intptr_t)chunk_ptr);

    next_chunk->in_use = false;
    // TODO: 
    
    next_chunk->size = chunk_ptr;
    next_chunk->prev = chunk_ptr;


    heap_info.avail_mem -= chunk_ptr->size;
    
    return chunk_ptr;


}

struct HeapChunk_t* get_next_free_chunk(size_t size) {
    struct HeapChunk_t* cur = heap_info.start_ptr;
    //TODO: req more if not enough
    while (cur) {
        if (!cur->in_use && cur->size > size) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void* get_chunk_data_ptr(struct HeapChunk_t* chunk) {
    intptr_t ptr = (intptr_t) chunk;
    ptr = (ptr + CHUNK_HEADER_SIZE + 16) & ~0xf;
    //fprintf(stderr, "chunk ptr at %x is div %d\n", ptr, IS_DIV_16(ptr));

    return (void*) ptr;
}
    

void print_chunk(struct HeapChunk_t* chunk) {
    fprintf(stderr, "chunk info - start %p next: %p prev %p in_use %d size %d data_ptr %p\n", 
            chunk, chunk->next, chunk->prev, chunk->in_use, chunk->size, 
            get_chunk_data_ptr(chunk));
    return;
}

void print_heap() {
    if (!heap_exists) {
        fprintf(stderr, "HEAP DOESNT EXISTS\n");
    }

    struct HeapChunk_t* cur = heap_info.start_ptr;
    fprintf(stderr, "\n[HEAP]\n start: %p avail_mem %lu\n", heap_info.start_ptr, heap_info.avail_mem);
    while (cur != NULL) {
        print_chunk(cur);
        cur = cur->next;
    }
}

