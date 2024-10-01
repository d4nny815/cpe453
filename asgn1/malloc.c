#include "malloc.h"

// TODO: fix assumption that header is 32B

static struct HeapInfo_t heap_info;
#define BUF_LEN (1500)
static char buf[BUF_LEN];

int init_heap() {
    intptr_t p_cur_end = (intptr_t)sbrk(0);
    intptr_t p_end_div_16 = make_div_16(p_cur_end);
    void* start_ptr = sbrk((p_end_div_16 - p_cur_end) + HEAP_INC_STEP);
    if (start_ptr == (void*)-1) {
        errno = ENOMEM;
        return -1;
    }

    heap_info.p_start = (HeapChunk_t*)start_ptr;
    heap_info.avail_mem = HEAP_INC_STEP - CHUNK_HEADER_SIZE; 
    heap_info.exists = true; 
        
    heap_info.p_start->next = NULL;
    heap_info.p_start->prev = NULL;
    heap_info.p_start->in_use = false;
    heap_info.p_start->size = HEAP_INC_STEP - CHUNK_HEADER_SIZE;

    return INIT_HEAP_PASSED;
}


void* malloc(size_t size) {
    snprintf(buf, BUF_LEN, "my malloc being called");
    puts(buf);
    if (!heap_info.exists) {
        if(init_heap() != INIT_HEAP_PASSED) {
            return NULL;
        }
    }
    HeapChunk_t* chunk_ptr = create_chunk(size);
    // print_chunk(chunk_ptr);
    return get_chunk_data_ptr(chunk_ptr);
}


void free(void* ptr) {
    // fprintf(stderr, "[FREE] %p\n", ptr);
    HeapChunk_t* p_chunk = heap_info.p_start;
    while ((intptr_t)p_chunk->next < (intptr_t)ptr) {
        p_chunk = p_chunk->next;
    }

    if (p_chunk == NULL) {
        //fprintf(stderr, "[FREE] not a valid ptr");
        return;
    }

    p_chunk->in_use = 0;
    heap_info.avail_mem += p_chunk->size;

    if (p_chunk->next != NULL && !p_chunk->next->in_use) { // merge with block behind
        HeapChunk_t* p_chunk_behind = p_chunk->next;
        //fprintf(stderr, "Merging %p with %p behind\n", p_chunk, p_chunk_behind);    

        p_chunk->next = p_chunk_behind->next;
        p_chunk_behind->prev = p_chunk;
        p_chunk->size += p_chunk_behind->size + CHUNK_HEADER_SIZE; // FIXME
        heap_info.avail_mem += CHUNK_HEADER_SIZE;
    } 
    if (p_chunk->prev != NULL && !p_chunk->prev->in_use) { // merge with block infront
        HeapChunk_t* p_chunk_infront = p_chunk->prev;
        //fprintf(stderr, "Merging %p with %p infront\n", p_chunk_infront, p_chunk);    
        p_chunk->next->prev = p_chunk_infront;
        p_chunk_infront->next = p_chunk->next;
        p_chunk_infront->size += p_chunk->size;
        heap_info.avail_mem += CHUNK_HEADER_SIZE;

    }

    return;
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        return NULL;
    }
    return memset(ptr, nmemb, size);
}

void* realloc(void* ptr, size_t size) {
    HeapChunk_t* p_chunk = get_pchunk_from_pdata(ptr);
    // FIXME: consider header size too
    // FIXME: consider if there is not enough space for header
    // TODO: test
    if (!p_chunk->next->in_use && (p_chunk->next->size + p_chunk->size) < (size)) {
        HeapChunk_t* p_next_chunk = p_chunk->next;
        size_t space_needed = size - p_chunk->size;
        p_chunk->size += space_needed;
        HeapChunk_t* p_new_next_chunk = (HeapChunk_t*)((intptr_t)p_next_chunk + (intptr_t)space_needed);
        p_new_next_chunk->next = p_next_chunk->next;
        p_chunk->next = p_new_next_chunk;
        p_new_next_chunk->prev = p_chunk;
        p_new_next_chunk->size = p_chunk->size - space_needed;
        p_new_next_chunk->in_use = false; 
    } 
    
    
    void* p_new = malloc(size);
    memcpy(p_new, ptr, p_chunk->size);
    free(ptr);
    return p_new;
}


HeapChunk_t* create_chunk(size_t size) {
    HeapChunk_t* p_chunk = get_free_chunk(size);
    if (p_chunk == NULL) {
        return NULL;
    }

    HeapChunk_t* p_next_chunk = get_chunk_data_ptr(p_chunk);
    p_next_chunk = (HeapChunk_t*)make_div_16((intptr_t)p_next_chunk + size);
    
    if (p_chunk->next == NULL) { // end of heap
        p_next_chunk->next = NULL;
        p_next_chunk->prev = p_chunk;
        p_next_chunk->in_use = false;
        p_next_chunk->size = (size_t)((intptr_t)sbrk(0) - (intptr_t)get_chunk_data_ptr(p_next_chunk));
    } else {
        // TODO: if not enough space for header give space to chunk 
        p_next_chunk->next = p_chunk->next;
        p_chunk->next->prev = p_next_chunk;
        p_next_chunk->prev = p_chunk; 
        p_next_chunk->in_use = false;
        p_next_chunk->size = (size_t)((intptr_t)p_next_chunk->next - (intptr_t)p_next_chunk);
    }
    
    p_chunk->next = p_next_chunk;
    p_chunk->size = (size_t)((intptr_t)p_next_chunk - (intptr_t)p_chunk);
    p_chunk->in_use = true;
    
    heap_info.avail_mem -= p_chunk->size;
     
    return p_chunk;
}


HeapChunk_t* get_free_chunk(size_t size) {
    HeapChunk_t* cur = heap_info.p_start;
    while (cur->next) {
        if (!cur->in_use && cur->size > size) {
            // fprintf(stderr, "free chunk at %p\n", cur);
            return cur;
        }
        cur = cur->next;
    }

    size_t inc_multiplier = size / HEAP_INC_STEP + 1;
    size_t inc_amount = inc_multiplier * HEAP_INC_STEP;
    void* old_end = sbrk(inc_amount);
    if (old_end == (void*)-1) {
        errno = ENOMEM;
        return NULL;
    }

    if (cur->next == NULL) {
        cur->size += inc_amount;
    } else {
        // TODO: prob doesnt work 
        HeapChunk_t* p_new_chunk = (HeapChunk_t*)old_end;
        p_new_chunk->next = NULL;
        p_new_chunk->prev = cur;
        cur->next = p_new_chunk;
        p_new_chunk->in_use = false;
        p_new_chunk->size = inc_amount;
    }
    
    heap_info.avail_mem += inc_amount;
    return cur;
}


void* get_chunk_data_ptr(HeapChunk_t* chunk) {
    intptr_t ptr = (intptr_t) chunk;
    ptr = make_div_16(ptr + CHUNK_HEADER_SIZE);
    return (void*)ptr;
}
    

HeapChunk_t* get_pchunk_from_pdata(void* ptr) {
    return (HeapChunk_t*)((intptr_t)ptr - CHUNK_HEADER_SIZE);
}

void print_chunk(HeapChunk_t* chunk) {
    snprintf(buf, BUF_LEN, "chunk info - p_start %p next: %p prev: %p"
            "in_use: %d size: %zu p_data: %p\n", 
            chunk, chunk->next, chunk->prev, chunk->in_use, chunk->size, 
            get_chunk_data_ptr(chunk));
    return;
}

void print_heap() {
    snprintf(buf, BUF_LEN, "printing heap");
    if (!heap_info.exists) {
        init_heap();
    }

    HeapChunk_t* cur = heap_info.p_start;
    void* end = sbrk(0);
    snprintf(buf, BUF_LEN, "\n[HEAP]\nstart: %p avail_mem %zu end %p\n", 
            heap_info.p_start, heap_info.avail_mem, end);
    while (cur != NULL) {
        print_chunk(cur);
        cur = cur->next;
    }
    snprintf(buf, BUF_LEN, "[END HEAP]\n\n");
}


size_t make_div_16(size_t val) {
    if (IS_DIV_16(val)) {
        return val;
    }
    return MAKE_DIV_16(val);
}
