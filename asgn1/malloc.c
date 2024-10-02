#include "malloc.h"

static struct HeapInfo_t heap_info;

#define BUF_LEN     (1500)
#define buf_len     (strlen(buf))
static char buf[BUF_LEN];


void* malloc(size_t size) {
  if (!heap_info.exists) {
    if(init_heap() != INIT_HEAP_PASSED) {
      // snprintf(buf, BUF_LEN, "[MALLOC] couldn't create mem\n");
      // write(STDERR_FILENO, buf, buf_len);
      return NULL;
    }
  }
  HeapChunk_t* p_chunk = get_free_chunk(size);
  if (p_chunk == NULL) {
    int mem_error = ask_more_mem(size);
    if (mem_error == HEAP_NOMEM_AVAIL) {
      return NULL;
    }
    p_chunk = heap_info.p_last;
  } else {
    split_chunk(p_chunk, size);
  }
  #ifdef DEBUG_MALLOC
    snprintf(buf, BUF_LEN, "MALLOC: malloc(%zu)    => (ptr=%p, size=%d)", 
            size, p_chunk, p_chunk->size);
    write(STDERR_FILENO, buf, len);
  #endif
  return get_chunk_data_ptr(p_chunk);
}


void* realloc(void* ptr, size_t size) {return NULL;}


void* calloc(size_t nmemb, size_t size) {return NULL;}


void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    return;
}


static int init_heap() {
  // TODO: ask for mem and check if div 16 good else make div 16;
  intptr_t p_cur_end = (intptr_t)sbrk(0);
  intptr_t p_end_div_16 = GET_DIV16_ADDR(p_cur_end);
  void* start_ptr = sbrk((p_end_div_16 - p_cur_end) + HEAP_INC_STEP);
  if (start_ptr == (void*)-1) {
    errno = ENOMEM;
    return HEAP_NOMEM_AVAIL;
  }

  heap_info.p_start = (HeapChunk_t*)start_ptr;
  heap_info.p_last = (HeapChunk_t*)start_ptr;
  heap_info.exists = true;
  heap_info.end_addr = (intptr_t) sbrk(0);

        
  heap_info.p_start->next = NULL;
  heap_info.p_start->prev = NULL;
  heap_info.p_start->in_use = false;
  heap_info.p_start->size = HEAP_INC_STEP - CHUNK_HEADER_SIZE;

  return INIT_HEAP_PASSED;
}


static HeapChunk_t* get_free_chunk(size_t size) {
  size_t req_size = GET_DIV16_ADDR(size);
  HeapChunk_t* p_cur = heap_info.p_start;
  do {
    if (!p_cur->in_use && p_cur->size >= req_size && 
        space_for_another_chunk(p_cur, req_size)) {
      return p_cur;
    }

  } while (p_cur->next && (p_cur = p_cur->next));

  return NULL;
}


static int ask_more_mem(size_t req_amt) {
  size_t inc_multiplier = req_amt / HEAP_INC_STEP + 1;
  size_t inc_amt = inc_multiplier * HEAP_INC_STEP;
  void* old_end = sbrk(inc_amt);
  if (old_end == (void*)-1) {
    errno = ENOMEM;
    return HEAP_NOMEM_AVAIL;
  }

  if (heap_info.p_last->in_use) {
    HeapChunk_t* p_new = (HeapChunk_t*) get_chunk_end_addr(heap_info.p_last);
    p_new->next = NULL;
    p_new->prev = heap_info.p_start;
    p_new->in_use = false;
    p_new->size = inc_amt;
    heap_info.p_last->next = p_new;
    heap_info.p_last = p_new;
  } else {
    heap_info.p_last->size += inc_amt;
  }
  return 0; // TODO: magic num

}


bool space_for_another_chunk(HeapChunk_t* chunk, size_t req_size) {
  intptr_t new_chunk_end_addr = GET_DIV16_ADDR(get_chunk_addr(chunk) + 
                                      req_size + MIN_CHUNK_SPACE);
  return new_chunk_end_addr < heap_info.end_addr;
}


static intptr_t get_chunk_end_addr(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return heap_info.end_addr - get_chunk_data_addr(chunk);
  }
  return get_chunk_addr(chunk->next) - get_chunk_addr(chunk);
}


static intptr_t get_chunk_data_ptr(HeapChunk_t* chunk) {
  return GET_DIV16_ADDR((intptr_t) chunk + CHUNK_HEADER_SIZE);
}


static size_t calc_user_chunk_size(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return (size_t)(heap_info.end_addr - get_chunk_data_addr(chunk));
  }
  return (size_t)(get_chunk_addr(chunk->next) - get_chunk_data_addr(chunk));
}


static size_t calc_tot_chunk_size(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return (size_t)(heap_info.end_addr - get_chunk_addr(chunk));
  }
  return (size_t)(get_chunk_addr(chunk->next) - get_chunk_addr(chunk));
}


static intptr_t get_chunk_addr(HeapChunk_t* chunk) {
  return (intptr_t) chunk;
}


static intptr_t get_chunk_data_addr(HeapChunk_t* chunk) {
  return (intptr_t) chunk + CHUNK_HEADER_SIZE;
}

