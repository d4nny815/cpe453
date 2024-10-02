#include "malloc.h"

static struct HeapInfo_t heap_info;
#define BUF_LEN (1500)
#define len     (strlen(buf))
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

    return INIT_HEAP_PASSED;
}


void* malloc(size_t size) {
    if (!heap_info.exists) {
        if(init_heap() != INIT_HEAP_PASSED) {
            snprintf(buf, BUF_LEN, "[MALLOC] couldn't create mem\n");
            write(STDERR, buf, len);
            return NULL;
        }
    }
    HeapChunk_t* p_chunk = get_free_chunk(size);
    if (p_chunk == NULL) {
        return NULL;
    }
    split_chunk(p_chunk, size);
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[MALLOC] ");
        write(STDERR, buf, len);
        print_chunk(p_chunk);
    #endif
    return get_chunk_data_ptr(p_chunk);
}


void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[FREE] %p\n", ptr);
        write(STDERR, buf, len);
    #endif
    HeapChunk_t* p_chunk = heap_info.p_start;
    while ((intptr_t)p_chunk->next < (intptr_t)ptr) {
        p_chunk = p_chunk->next;
    }

    if (p_chunk == NULL) {
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[FREE] not a valid ptr\n");
            write(STDERR, buf, len);
        #endif
        return;
    }

    p_chunk->in_use = 0;
    heap_info.avail_mem += p_chunk->size;

    // merge with block behind
    if (p_chunk->next != NULL && !p_chunk->next->in_use) { 
        HeapChunk_t* p_chunk_behind = p_chunk->next;
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "Merging %p with %p behind\n", 
                     p_chunk, p_chunk_behind);
            write(STDERR, buf, len);    
        #endif

        p_chunk->next = p_chunk_behind->next;
        p_chunk_behind->prev = p_chunk;
        p_chunk->size += p_chunk_behind->size + CHUNK_HEADER_SIZE; // FIXME
        heap_info.avail_mem += CHUNK_HEADER_SIZE;
    } 
    // merge with block infront
    if (p_chunk->prev != NULL && !p_chunk->prev->in_use) { 
        HeapChunk_t* p_chunk_infront = p_chunk->prev;
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "Merging %p with %p infront\n", 
                   p_chunk_infront, p_chunk);
            write(STDERR, buf, len);    
        #endif
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

    if (size == 0) {
        free(ptr);
        return NULL;
    }
    // TODO: check if ptr is in the heap
    HeapChunk_t* p_chunk = get_pchunk_from_pdata(ptr);
    #ifdef DEBUG_MALLOC
#endif
    //print_chunk(p_chunk);
    //print_chunk(p_chunk->next);
    size_t cur_size_and_next = (CHUNK_HEADER_SIZE + p_chunk->size +
                                p_chunk->next->size);
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[REALLOC] req %zu combined size %zu chunk_size"
               " %zu next_chunk_size %zu\n", size, cur_size_and_next, 
                p_chunk->size, p_chunk->next->size);
        write(STDERR, buf, len);
    #endif
    if (size < p_chunk->size) {
        //print_chunk(p_chunk);
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[REALLOC] shrinking %zu down to %zu\n", 
                  p_chunk->size, size);
            write(STDERR, buf, len);
        #endif
        split_chunk(p_chunk, size);
        //print_chunk(p_chunk);
        //print_chunk(p_chunk->next);
        return get_chunk_data_ptr(p_chunk);
    }
    // TODO: add case for more mem
    else if (!p_chunk->next->in_use && size < cur_size_and_next){
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[REALLOC] realloc shouldnt move buffer\n");
            write(STDERR, buf, len);
        #endif
        // TODO: if chunk is at end of heap
        // TODO: if next next chunk is NULL
        // There exist 2 chunks after user chunk
        if (p_chunk->next != NULL && p_chunk->next->next != NULL) {
            p_chunk->next = p_chunk->next->next;
            p_chunk->next->next->prev = p_chunk;
        }
        // FIXME: do I need to update size
        split_chunk(p_chunk, size);
        return get_chunk_data_ptr(p_chunk);
    } 
    
    
    void* p_new = malloc(size);
    memcpy(p_new, ptr, p_chunk->size);
    free(ptr);
    return p_new;
}


HeapChunk_t* get_free_chunk(size_t size) {
    HeapChunk_t* cur = heap_info.p_start;
    intptr_t end_addr = (intptr_t) sbrk(0);
    do {
        if (!cur->in_use && cur->size > (size + MIN_CHUNK_SPACE) &&
            make_div_16((intptr_t) cur + size + MIN_CHUNK_SPACE) < 
            end_addr) {
            #ifdef DEBUG_MALLOC
                snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] free chunk at %p"
                       " for size %zu\n", cur, size);
                write(STDERR, buf, len);
            #endif
            return cur;
        }
    } while (cur->next && (cur = cur->next));

    if (cur == NULL) {
        cur = heap_info.p_start;
    }
    
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] asking for more mem\n");
        write(STDERR, buf, len);
    #endif

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
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] adding more"
                   " space to end of heap\n");
            write(STDERR, buf, len);
        #endif
    } 
    // create a new free chunk at the end of the heap for user
    else {
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[GET_FREE_CHUNK] creating a new "
                   " chunk at end of heap\n");
            write(STDERR, buf, len);
        #endif
        HeapChunk_t* p_new_chunk = (HeapChunk_t*)old_end;
        p_new_chunk->next = NULL;
        p_new_chunk->prev = cur;
        cur->next = p_new_chunk;
        p_new_chunk->in_use = false;
        p_new_chunk->size = inc_amount;
    }
    
    return cur;
}


void split_chunk(HeapChunk_t* chunk, size_t size) {
    intptr_t chunk_addr = (intptr_t) chunk;
    intptr_t new_chunk_addr = (intptr_t) GET_DIV16_ADDR(CHUNK_HEADER_SIZE + 
                                                    chunk_addr + size);
    intptr_t next_chunk_addr = (intptr_t) chunk->next;

    HeapChunk_t* p_new_chunk = (HeapChunk_t*) new_chunk_addr; 
    HeapChunk_t* p_next_chunk = (HeapChunk_t*) next_chunk_addr;
    size_t chunk_size = new_chunk_addr - chunk_addr - CHUNK_HEADER_SIZE;
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[SPLIT_CHUNK] chunk size %p %p header size:"
           " %zu %zu\n", p_new_chunk, chunk, CHUNK_HEADER_SIZE, chunk_size);
        write(STDERR, buf, len);
    #endif
    if (chunk->next == NULL) {
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[SPLIT_CHUNK] adding to end of heap\n");
            write(STDERR, buf, len);
        #endif
        p_new_chunk->next = NULL;
        p_new_chunk->prev = chunk;
        p_new_chunk->size = (intptr_t) sbrk(0) - new_chunk_addr + 
                            CHUNK_HEADER_SIZE;
        p_new_chunk->in_use = false;
        chunk->next = p_new_chunk;
        chunk->size = chunk_size;
        chunk->in_use = true;
        return;
    }
    
    if (next_chunk_addr - new_chunk_addr > 
        size + MIN_CHUNK_SPACE) {
        #ifdef DEBUG_MALLOC
            snprintf(buf, BUF_LEN, "[SPLIT_CHUNK] sandwiched with "
                   " room req: %zu\n", size);
            write(STDERR, buf, len);
        #endif
        
        p_new_chunk->next = p_next_chunk;
        chunk->next = p_new_chunk;
        p_new_chunk->prev = chunk;
        p_new_chunk->size = next_chunk_addr - new_chunk_addr + 
                            CHUNK_HEADER_SIZE;
        p_new_chunk->in_use = false;
        p_next_chunk->prev = p_new_chunk;
        chunk->size = chunk_size;
        chunk->in_use = true;
        return;
    }
    #ifdef DEBUG_MALLOC
        snprintf(buf, BUF_LEN, "[SPLIT_CHUNK] sandwiched with no room\n");
        write(STDERR, buf, len);
    #endif
    return;
}

    // create a new header
void* get_chunk_data_ptr(HeapChunk_t* chunk) {
    intptr_t ptr = (intptr_t) chunk;
    ptr = make_div_16(ptr + CHUNK_HEADER_SIZE);
    return (void*)ptr;
}
    

HeapChunk_t* get_pchunk_from_pdata(void* ptr) {
    return (HeapChunk_t*)((intptr_t)ptr - CHUNK_HEADER_SIZE);
}


bool space_for_another_chunk(HeapChunk_t* chunk, size_t size) {
    intptr_t cur_chunk_addr = (intptr_t) get_chunk_data_ptr(chunk);
    intptr_t next_chunk_addr = (intptr_t) chunk->next;
    intptr_t size_needed = (next_chunk_addr - cur_chunk_addr);
    snprintf(buf, BUF_LEN, "[SPACE_ANOTHER_CHUNK] size asked %zu, "
            " needed %x\n", size, size_needed);
    write(STDERR, buf, len);
    return size_needed > MIN_CHUNK_SPACE + size;
}


void print_chunk(HeapChunk_t* chunk) {
    snprintf(buf, BUF_LEN, "chunk info - p_start %p next: %p prev: %p"
            " in_use: %d size: %zu p_data: %p\n", 
            chunk, chunk->next, chunk->prev, chunk->in_use, chunk->size, 
            get_chunk_data_ptr(chunk));
    write(STDERR, buf, len);
    return;
}

void print_heap() {
    if (!heap_info.exists) {
        init_heap();
    }

    HeapChunk_t* cur = heap_info.p_start;
    void* end = sbrk(0);
    snprintf(buf, BUF_LEN, "\n[HEAP]\nstart: %p avail_mem %zu end %p\n", 
            heap_info.p_start, heap_info.avail_mem, end);
    write(STDERR, buf, len);
    while (cur != NULL) {
        print_chunk(cur);
        cur = cur->next;
    }
    snprintf(buf, BUF_LEN, "[END HEAP]\n\n");
    write(STDERR, buf, len); 

    return;
}


size_t make_div_16(size_t val) {
    if (val % 16) {
       return (val + 16) & ~0xf; 
    }
    return val;
}
