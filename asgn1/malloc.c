#include "malloc.h"

static struct HeapInfo_t heap_info;

void init_heap() {
    // TODO: error check sbrk
    void* start_ptr = sbrk(HEAP_INC_AMOUNT); // FIXME: maybe sbrk(0) so i dont break
    heap_info.p_start = (HeapChunk_t*)start_ptr;
    heap_info.avail_mem = HEAP_INC_AMOUNT - CHUNK_HEADER_SIZE; 
    heap_info.exists = true; 
        
    heap_info.p_start->next = NULL;
    heap_info.p_start->prev = NULL;
    heap_info.p_start->in_use = false;
    heap_info.p_start-> size = HEAP_INC_AMOUNT - CHUNK_HEADER_SIZE;

    //fprintf(stderr, "heap starts at %p with %ld free mem\n", 
    //        heap_info.start_ptr, heap_info.avail_mem);   
    //print_chunk(heap_info.start_ptr); 
    //fprintf(stderr, "data starts at %p\n", get_chunk_data_ptr(heap_info.start_ptr));
    return;
}


void* mymalloc(size_t size) {
    if (!heap_info.exists) {
        init_heap();
    }
    HeapChunk_t* chunk_ptr = create_chunk(size);

    // TODO: split a free chunk smaller when inbetween used blocks 

    return get_chunk_data_ptr(chunk_ptr);
}

// TODO: ptr can be anywhere in the chunk
// prob traverse and go until chunk ptr is greater then ptr
void myfree(void* ptr) {
    HeapChunk_t* p_chunk = (HeapChunk_t*)(((intptr_t) ptr - CHUNK_HEADER_SIZE) & ~0xf);
    print_chunk(p_chunk);
    p_chunk->in_use = 0;
    heap_info.avail_mem += p_chunk->size;
    return;
}

HeapChunk_t* create_chunk(size_t size) {
    HeapChunk_t* p_chunk = get_next_free_chunk(size);

    HeapChunk_t* p_next_chunk = get_chunk_data_ptr(p_chunk);
    p_next_chunk = (HeapChunk_t*)MAKE_DIV_16((intptr_t)p_next_chunk + size);
    
    
    p_chunk->next = p_next_chunk;
    p_chunk->in_use = true;
    p_chunk->size = (size_t)((intptr_t)p_next_chunk - (intptr_t)p_chunk);

    heap_info.avail_mem -= p_chunk->size;

    p_next_chunk->prev = p_chunk;
    p_next_chunk->size = heap_info.avail_mem; // TODO: calc size correctly
    p_next_chunk->in_use = false;
    
    return p_chunk;
}

HeapChunk_t* get_next_free_chunk(size_t size) {
    HeapChunk_t* cur = heap_info.p_start;
    while (cur) {
        if (!cur->in_use && cur->size > size) {
            //fprintf(stderr, "free chunk at %p\n", cur);
            return cur;
        }
        cur = cur->next;
    }
    //TODO: req more if not enough also check avail mem
    return NULL;
}

void* get_chunk_data_ptr(HeapChunk_t* chunk) {
    intptr_t ptr = (intptr_t) chunk;
    ptr = MAKE_DIV_16(ptr + CHUNK_HEADER_SIZE);
    // ptr = (ptr + CHUNK_HEADER_SIZE + 16) & ~0xf;
    //fprintf(stderr, "chunk ptr at %x is div %d\n", ptr, IS_DIV_16(ptr));

    return (void*) ptr;
}
    

void print_chunk(HeapChunk_t* chunk) {
    fprintf(stderr, 
            "chunk info - p_start %p next: %p prev: %p in_use: %d size: %d p_data: %p\n", 
            chunk, chunk->next, chunk->prev, chunk->in_use, chunk->size, 
            get_chunk_data_ptr(chunk));
    return;
}

void print_heap() {
    if (!heap_info.exists) {
        init_heap();
    }

    HeapChunk_t* cur = heap_info.p_start;
    fprintf(stderr, "\n[HEAP]\nstart: %p avail_mem %lu\n", heap_info.p_start, heap_info.avail_mem);
    while (cur != NULL) {
        print_chunk(cur);
        cur = cur->next;
    }
    fprintf(stderr, "[END HEAP]\n\n");
}

