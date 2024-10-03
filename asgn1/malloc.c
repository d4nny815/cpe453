#include "malloc.h"

static struct HeapInfo_t heap_info;

#define BUF_LEN     (1500)
#define buf_len     (strlen(buf))
static char buf[BUF_LEN];


void* malloc(size_t size) {
  if (!heap_info.exists) {
    if(init_heap() != INIT_HEAP_PASSED) {
      snprintf(buf, BUF_LEN, "[MALLOC] couldn't create mem\n");
      write(STDERR_FILENO, buf, buf_len);
      return NULL;
    }
  }
  size_t req_size = GET_DIV16_ADDR(size);
  if (req_size == 0) {
    return  NULL;
  }
  HeapChunk_t* p_chunk = get_free_chunk(req_size);
  if (p_chunk == NULL) {
    int mem_error = ask_more_mem(req_size);
    if (mem_error == HEAP_NOMEM_AVAIL) {
      return NULL;
    }
    p_chunk = heap_info.p_last;
  }
  split_chunk(p_chunk, req_size);
  return (void*)get_chunk_data_addr(p_chunk);
}


void* realloc(void* ptr, size_t size) {
  size_t req_size = GET_DIV16_ADDR(size);
  if (size == 0) {
    free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    return malloc(size);
  }

  HeapChunk_t* chunk = get_pchunk_from_pdata(ptr);
  size_t og_size = chunk->size;
  if (chunk->next->size > HEAP_INC_STEP << 5) {
    printf("FAILED");
    exit(1);
  }

  // TODO: shrink
  if (req_size <= chunk->size) {
    // TODO: if not enough for header do nothing
    if (!space_for_another_chunk(chunk, req_size)) {
      return ptr;
    }
    split_chunk(chunk, req_size);
  }

  // TODO: stay
  // end of heap with room -> split
  // end of heap with no room -> ask more then split
  // 2nd end of heap with room -> split
  // 2nd end of heap with no room -> ask more then split
  // inbetween with space -> split
  else if (chunk->next == NULL) {
    if (!space_for_another_chunk(chunk, req_size)) {
      // ask for more mem
      int mem_error = ask_more_mem(req_size);
      if (mem_error == HEAP_NOMEM_AVAIL) {
        return NULL;
      }
    }
    split_chunk(chunk, req_size);
  }

  else if (chunk->next->next == NULL) {
    if (!space_for_another_chunk(chunk, req_size)) {
      int mem_error = ask_more_mem(req_size);
      if (mem_error == HEAP_NOMEM_AVAIL) {
        return NULL;
      }
    }
    chunk->size += calc_tot_chunk_size(chunk->next);
    chunk->next = NULL;
    split_chunk(chunk, req_size);
  }

  else if (!chunk->next->in_use && req_size <= chunk->size + 
          calc_tot_chunk_size(chunk->next) + MIN_CHUNK_SPACE) {
    chunk->next = chunk->next->next;
    chunk->next->next->prev = chunk;
    chunk->size = calc_user_chunk_size(chunk);
    split_chunk(chunk, req_size);
  }
  
  
  // TODO: move
  // inbetween with no room -> malloc(req_size) then free(og)
  else {
    void* p_new = malloc(req_size);
    memmove(p_new, ptr, og_size);
    free(ptr);
    return p_new;
  }
  
  
  return ptr; 
}


void* calloc(size_t nmemb, size_t size) {
  size_t actual_size = nmemb * size;
  void* ptr = malloc(actual_size);
  if (ptr == NULL) {
    return NULL;
  } 
  return memset(ptr, 0, actual_size);
}


void free(void* ptr) {
  if (ptr == NULL) {
    return;
  }

  HeapChunk_t* p_cur = heap_info.p_start;
  do {
    if (ptr_in_chunk(ptr, p_cur)) {
      break;
    }
    p_cur = p_cur->next;
  } while (p_cur);

  if (p_cur == NULL) {
    return;
  }

  if (!p_cur->in_use) {
    return;
  }


  p_cur->in_use = false;

  if (p_cur->prev != NULL && !p_cur->prev->in_use) {
    HeapChunk_t* prev_chunk = p_cur->prev;
    prev_chunk->next = p_cur->next;
    p_cur->next->prev = prev_chunk;
    prev_chunk->size = calc_user_chunk_size(prev_chunk);
  }

  if (p_cur->next != NULL && !p_cur->next->in_use) {
    HeapChunk_t* next_chunk = p_cur->next;
    p_cur->next = next_chunk->next;
    if (next_chunk->next != NULL) {
      next_chunk->next->prev = p_cur;
    }
    p_cur->size = calc_user_chunk_size(p_cur);
  }

  return;
}


int init_heap() {
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


HeapChunk_t* get_free_chunk(size_t size) {
  HeapChunk_t* p_cur = heap_info.p_start;
  do {
    if (!p_cur->in_use && p_cur->size >= size && 
        space_for_another_chunk(p_cur, size)) {
      return p_cur;
    }
    p_cur = p_cur->next;
  } while (p_cur);

  return NULL;
}


void split_chunk(HeapChunk_t* chunk, size_t size) {
  chunk->size = size;
  HeapChunk_t* p_new_chunk = (HeapChunk_t*) (get_chunk_data_addr(chunk) +
                                            size);
  HeapChunk_t* p_next_chunk = chunk->next;

  if (chunk->next == NULL) {
    p_new_chunk->next = NULL;
    p_new_chunk->prev = chunk;
    chunk->next = p_new_chunk;
    p_new_chunk->size = calc_user_chunk_size(p_new_chunk);
    heap_info.p_last = p_new_chunk;
  } else {
    p_new_chunk->next = p_next_chunk;
    p_new_chunk->prev = chunk;
    chunk->next = p_new_chunk;
    p_next_chunk->prev = p_new_chunk;
    p_new_chunk->size = calc_user_chunk_size(p_new_chunk);
  }
  
  p_new_chunk->in_use = false;
  chunk->in_use = true;

  return;
}


int ask_more_mem(size_t req_amt) {
  size_t inc_multiplier = req_amt / HEAP_INC_STEP + 1;
  size_t inc_amt = inc_multiplier * HEAP_INC_STEP;
  void* old_end = sbrk(inc_amt);
  if (old_end == (void*)-1) {
    errno = ENOMEM;
    return HEAP_NOMEM_AVAIL;
  }

  heap_info.end_addr += inc_amt;

  if (heap_info.p_last->in_use) {
    HeapChunk_t* p_new = (HeapChunk_t*)get_chunk_end_addr(
                                                        heap_info.p_last);
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
  intptr_t new_chunk_end_addr = get_chunk_data_addr(chunk);
  new_chunk_end_addr += req_size;
  new_chunk_end_addr += CHUNK_HEADER_SIZE;
  if (chunk->next == NULL) {
    return new_chunk_end_addr < heap_info.end_addr;
  }
  intptr_t next_chunk_addr = get_chunk_addr(chunk->next);
  intptr_t cur_space = next_chunk_addr - new_chunk_end_addr;
  bool is_space = cur_space > 32;
  return is_space;
}


bool ptr_in_chunk(void* ptr, HeapChunk_t* chunk) {
  intptr_t ptr_addr = (intptr_t) ptr;
  intptr_t chunk_data_addr = get_chunk_data_addr(chunk);
  intptr_t chunk_end_addr = get_chunk_end_addr(chunk);
  return chunk_data_addr <= ptr_addr && ptr_addr < chunk_end_addr; 
}


intptr_t get_chunk_end_addr(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return heap_info.end_addr;
  }
  return get_chunk_addr(chunk->next);
}


size_t calc_user_chunk_size(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return (size_t)(heap_info.end_addr - get_chunk_data_addr(chunk));
  }
  return (size_t)(get_chunk_addr(chunk->next) - get_chunk_data_addr(chunk));
}


size_t calc_tot_chunk_size(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return (size_t)(heap_info.end_addr - get_chunk_addr(chunk));
  }
  return (size_t)(get_chunk_addr(chunk->next) - get_chunk_addr(chunk));
}


intptr_t get_chunk_addr(HeapChunk_t* chunk) {
  return (intptr_t) chunk;
}


intptr_t get_chunk_data_addr(HeapChunk_t* chunk) {
  return (intptr_t) chunk + CHUNK_HEADER_SIZE;
}


HeapChunk_t* get_pchunk_from_pdata(void* ptr) {
  intptr_t ptr_addr = (intptr_t) ptr;
  ptr_addr -= CHUNK_HEADER_SIZE;
  return (HeapChunk_t*) ptr_addr;
}


void print_heap() {
  if (!getenv("DEBUG_MALLOC_ME")) {
    return;
  }
  if (!heap_info.exists) {
    init_heap();
  }
  snprintf(buf, BUF_LEN, "\n[HEAP]\nstart_addr %p end_addr %p, last"
          " chunk_addr %p\n", heap_info.p_start, 
          (void*)heap_info.end_addr, heap_info.p_last);
  write(STDERR_FILENO, buf, buf_len);
  
  HeapChunk_t* cur = heap_info.p_start;

  while (cur != NULL) {
    print_chunk(cur);
    cur = cur->next;
  }
  snprintf(buf, BUF_LEN, "[END_HEAP]\n\n");
  write(STDERR_FILENO, buf, buf_len);
  return;
}


void print_chunk(HeapChunk_t* chunk) {
  if (!getenv("DEBUG_MALLOC_ME")) {
    return;
  }
  snprintf(buf, BUF_LEN, "[CHUNK] start_addr %p next %p prev %p" 
          " tot size %zu user size %zu used %d data_addr %p\n", chunk, 
          chunk->next, chunk->prev, calc_tot_chunk_size(chunk), 
          calc_user_chunk_size(chunk), chunk->in_use, 
          (void*)get_chunk_data_addr);
  write(STDERR_FILENO, buf, buf_len);
  return;
}
