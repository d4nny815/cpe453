#include "malloc.h"

static struct HeapInfo_t heap_info;
#define BUF_LEN (1500)
static char buf[BUF_LEN];

int init_heap() {
    // start heap at addr div by 16
    intptr_t p_cur_end = (intptr_t)sbrk(0);
    intptr_t p_end_div_16 = make_div_16(p_cur_end);
    void* start_ptr = sbrk((p_end_div_16 - p_cur_end) + HEAP_INC_STEP);
    if (start_ptr == (void*)-1) {
        errno = ENOMEM;
        return HEAP_NOMEM_AVAIL;
    }

    heap_info.p_start = (HeapChunk_t*)start_ptr;
    heap_info.avail_mem = HEAP_INC_STEP - CHUNK_HEADER_SIZE; 
    heap_info.exists = true; 
        
    heap_info.p_start->next = NULL;
    heap_info.p_start->prev = NULL;
    heap_info.p_start->in_use = false;
    heap_info.p_start->size = HEAP_INC_STEP - CHUNK_HEADER_SIZE;

    //print_chunk(heap_info.p_start);
    return INIT_HEAP_PASSED;
}


void* malloc(size_t size) {
    if (!heap_info.exists) {
        if(init_heap() != INIT_HEAP_PASSED) {
            snprintf(buf, BUF_LEN, "[MALLOC] couldn't create mem");
            puts(buf);
            return NULL;
        }
    }
    HeapChunk_t* chunk_ptr = create_chunk(size);
    //print_chunk(chunk_ptr);
    return get_chunk_data_ptr(chunk_ptr);
}


void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
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

    // merge with block behind
    if (p_chunk->next != NULL && !p_chunk->next->in_use) { 
        HeapChunk_t* p_chunk_behind = p_chunk->next;
        //fprintf(stderr, "Merging %p with %p behind\n", 
        //          p_chunk, p_chunk_behind);    

        p_chunk->next = p_chunk_behind->next;
        p_chunk_behind->prev = p_chunk;
        p_chunk->size += p_chunk_behind->size + CHUNK_HEADER_SIZE; // FIXME
        heap_info.avail_mem += CHUNK_HEADER_SIZE;
    } 
    // merge with block infront
    if (p_chunk->prev != NULL && !p_chunk->prev->in_use) { 
        HeapChunk_t* p_chunk_infront = p_chunk->prev;
        //fprintf(stderr, "Merging %p with %p infront\n", 
        //        p_chunk_infront, p_chunk);    
        p_chunk->next->prev = p_chunk_infront;
        p_chunk_infront->next = p_chunk->next;
        p_chunk_infront->size += p_chunk->size;
        heap_info.avail_mem += CHUNK_HEADER_SIZE;

    }

    return;
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(size * nmemb);
    if (ptr == NULL) {
        return NULL;
    }
    return memset(ptr, 0, size * nmemb);
}

void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    
    }
    HeapChunk_t* p_chunk = get_pchunk_from_pdata(ptr);
    // FIXME: consider header size too
    // FIXME: consider if there is not enough space for header
    // TODO: realloc on NULL with a size
    // TODO: test
    if (!p_chunk->next->in_use && 
        (p_chunk->next->size + p_chunk->size) < (size)) {
        HeapChunk_t* p_next_chunk = p_chunk->next;
        size_t space_needed = size - p_chunk->size;
        p_chunk->size += space_needed;
        HeapChunk_t* p_new_next_chunk = (HeapChunk_t*)\
                    ((intptr_t)p_next_chunk + (intptr_t)space_needed);
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
    
    // free chunk is at end of heap
    if (p_chunk->next == NULL) { 
        //snprintf(buf, BUF_LEN, "[CREATE_CHUNK] free chunk at heap end\n");
        //puts(buf);
        p_next_chunk->next = NULL;
        p_next_chunk->prev = p_chunk;
        p_next_chunk->in_use = false;
        p_next_chunk->size = (size_t)((intptr_t)sbrk(0) - \
            (intptr_t)get_chunk_data_ptr(p_next_chunk));
    }
    // not enough space for a new chunk
    //else if () {
        
    //}
    // free chunk is inbetween chunks
    else {
        snprintf(buf, BUF_LEN, "[CREATE_CHUNK] free chunk sandwiched");
        puts(buf);
        p_next_chunk->next = p_chunk->next; 
        p_chunk->next->prev = p_next_chunk;
        p_next_chunk->prev = p_chunk; 
        p_next_chunk->in_use = false;
        p_next_chunk->size = (size_t)\
            ((uintptr_t)p_next_chunk->next - (uintptr_t)p_next_chunk);
    }

    p_chunk->next = p_next_chunk;
    p_chunk->size = (size_t)((uintptr_t)p_next_chunk - (uintptr_t)p_chunk);
    p_chunk->in_use = true;
    
    heap_info.avail_mem -= p_chunk->size;
    //print_chunk(p_chunk); 
    //snprintf(buf, BUF_LEN, "heap is %zu, %zu p_chunk %p, p_next %p", 
    //        heap_info.avail_mem, p_chunk->size, p_chunk, p_next_chunk);
    //puts(buf);
    //print_chunk(p_next_chunk);
    return p_chunk;
}


HeapChunk_t* get_free_chunk(size_t size) {
    HeapChunk_t* cur = heap_info.p_start;
    while (cur) {
        if (!cur->in_use && cur->size > size) {
            //snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] free chunk at %p", 
            //cur);
            //puts(buf);
            return cur;
        }
        cur = cur->next;
    }

    cur = cur->prev;
    
    if (cur == heap_info.p_start) {
        return cur;
    }
   
    snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] asking for more mem");
    puts(buf);
    size_t inc_multiplier = size / HEAP_INC_STEP + 1;
    size_t inc_amount = inc_multiplier * HEAP_INC_STEP;
    void* old_end = sbrk(inc_amount);
    if (old_end == (void*)-1) {
        errno = ENOMEM;
        return NULL;
    }

    // added more space to end of heap
    if (cur->next == NULL) {
        cur->size += inc_amount;
    } 
    // create a new free chunk at the end of the heap for user
    else {
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
            " in_use: %d size: %zu p_data: %p", 
            chunk, chunk->next, chunk->prev, chunk->in_use, chunk->size, 
            get_chunk_data_ptr(chunk));
    puts(buf);
    return;
}

void print_heap() {
    if (!heap_info.exists) {
        init_heap();
    }

    HeapChunk_t* cur = heap_info.p_start;
    void* end = sbrk(0);
    snprintf(buf, BUF_LEN, "\n[HEAP]\nstart: %p avail_mem %zu end %p", 
            heap_info.p_start, heap_info.avail_mem, end);
    puts(buf);
    while (cur != NULL) {
        print_chunk(cur);
        cur = cur->next;
    }
    snprintf(buf, BUF_LEN, "[END HEAP]\n");
    puts(buf);
    
    return;
}


size_t make_div_16(size_t val) {
    if (val % 16) {
       return (val + 16) & ~0xf; 
    }
    return val;
}
