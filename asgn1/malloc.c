#include "malloc.h"

static struct HeapInfo_t heap_info;

static int init_heap();
static HeapChunk_t* get_free_chunk(size_t size);
static int ask_more_mem(size_t req_amt);
static void split_chunk(HeapChunk_t* chunk, size_t size);
static bool space_for_another_chunk(HeapChunk_t* chunk, size_t req_size);
static bool ptr_in_chunk(void* ptr, HeapChunk_t* chunk);
static size_t calc_user_chunk_size(HeapChunk_t* chunk);
static size_t calc_tot_chunk_size(HeapChunk_t* chunk);
static intptr_t get_chunk_addr(HeapChunk_t* chunk);
static intptr_t get_chunk_data_addr(HeapChunk_t* chunk);
static intptr_t get_chunk_end_addr(HeapChunk_t* chunk);
static HeapChunk_t* get_pchunk_from_pdata(void* ptr);

#define BUF_LEN     (1500)
#define buf_len     (strlen(buf))
static char buf[BUF_LEN];


void* malloc(size_t size) {
  
  if (!heap_info.exists) {
    if(init_heap() != INIT_HEAP_PASSED) {
      int err = snprintf(buf, BUF_LEN, "[MALLOC] NO memory available\n");
      if (err < 0) {
        exit(1);
      }
      err = write(STDERR_FILENO, buf, buf_len);
      if (err == -1) {
        exit(1);
      }
      return NULL;
    }
  }

  size_t req_size = GET_DIV16_VAL(size);
  if (req_size < 0) {
    // if (getenv("DEBUG_MALLOC")) {
    //   int err = snprintf(buf, BUF_LEN, "MALLOC: malloc(%zu)      => (ptr=%p, size=%zu)\n", size, NULL, req_size);
    //   if (err < 0) {
    //     exit(1);
    //   }
    //   err = write(STDERR_FILENO, buf, buf_len);
    //   if (err == -1) {
    //     exit(1);
    //   }
    // }
    return  NULL;
  }

  HeapChunk_t* p_chunk = get_free_chunk(req_size);
  if (p_chunk == NULL) {
    int mem_error = ask_more_mem(req_size);
    if (mem_error == HEAP_NOMEM_AVAIL) {
      int err = snprintf(buf, BUF_LEN, "[MALLOC] NO memory available\n");
      if (err < 0) {
        exit(1);
      }
      err = write(STDERR_FILENO, buf, buf_len);
      if (err == -1) {
        exit(1);
      }
      return NULL;
    }
    p_chunk = heap_info.p_last;
  }

  split_chunk(p_chunk, req_size);
  
  // if (getenv("DEBUG_MALLOC")) {
  //   int err = snprintf(buf, BUF_LEN, "MALLOC: malloc(%zu)      => (ptr=%p, size=%zu)\n", size, p_chunk, req_size);
  //   if (err < 0) {
  //     exit(1);
  //   }
  //   err = write(STDERR_FILENO, buf, buf_len);
  //   if (err == -1) {
  //     exit(1);
  //   }
  // }

  return (void*)get_chunk_data_addr(p_chunk);
}


void* realloc(void* ptr, size_t size) {
  size_t req_size = GET_DIV16_VAL(size);
  if (size == 0) {
    free(ptr);
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, ptr, size);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return NULL;
  }

  if (ptr == NULL) {
    ptr = malloc(size);
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, ptr, 
                        // get_pchunk_from_pdata(ptr)->size);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return ptr;
  }

  HeapChunk_t* chunk = get_pchunk_from_pdata(ptr);
  size_t og_size = chunk->size;

  // (shrink)
  if (req_size < chunk->size && space_for_another_chunk(chunk, req_size)) {
    split_chunk(chunk, req_size);
  }

  else if (req_size < chunk->size) {
    // not enough room for another chunk so do nothing
    chunk->size = GET_DIV16_VAL(size);
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, chunk, 
                        // chunk->size);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return ptr;
  }

  // chunk is at end of heap (stay)
  else if (chunk->next == NULL) {
    if (!space_for_another_chunk(chunk, req_size)) {
      int mem_error = ask_more_mem(req_size);
      if (mem_error == HEAP_NOMEM_AVAIL) {
        int err = snprintf(buf, BUF_LEN, "[MALLOC] NO memory available\n");
        if (err < 0) {
          exit(1);
        }
        err = write(STDERR_FILENO, buf, buf_len);
        if (err == -1) {
          exit(1);
        }
        return NULL;
      }
    }
    split_chunk(chunk, req_size);
  }

  // chunk is 2 to last at end of heap (stay)
  else if (chunk->next->next == NULL) {
    if (!space_for_another_chunk(chunk, req_size)) {
      int mem_error = ask_more_mem(req_size);
      if (mem_error == HEAP_NOMEM_AVAIL) {
        int err = snprintf(buf, BUF_LEN, "[MALLOC] NO memory available\n");
        if (err < 0) {
          exit(1);
        }
        err = write(STDERR_FILENO, buf, buf_len);
        if (err == -1) {
          exit(1);
        }
        return NULL;
      }
    }
    chunk->size += calc_tot_chunk_size(chunk->next);
    chunk->next = NULL;
    split_chunk(chunk, req_size);
  }

  // chunk is sandwiched and has enough space to split (stay)
  else if (!chunk->next->in_use && req_size <= chunk->size + 
          calc_tot_chunk_size(chunk->next) + MIN_CHUNK_SPACE) {
    chunk->next = chunk->next->next;
    chunk->next->next->prev = chunk;
    chunk->size = calc_user_chunk_size(chunk);
    split_chunk(chunk, req_size);
  }
  
  // inbetween with no room (move)
  else {
    void* p_new = malloc(req_size);
    memmove(p_new, ptr, og_size);
    free(ptr);
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, 
                        // get_pchunk_from_pdata(p_new), 
                        // get_pchunk_from_pdata(p_new)->size);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return p_new;
  }
  
  // if (getenv("DEBUG_MALLOC")) {
    // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, 
                      // get_pchunk_from_pdata(ptr), 
                      // get_pchunk_from_pdata(ptr)->size);
    // if (err < 0) {
      // exit(1);
    // }
    // err = write(STDERR_FILENO, buf, buf_len);
    // if (err == -1) {
      // exit(1);
    // }
  // }
  return ptr; 
}


void* calloc(size_t nmemb, size_t size) {
  size_t actual_size = nmemb * size;
  void* ptr = malloc(actual_size);
  if (ptr == NULL) {
    // if (getenv("DEBUG_MALLOC")) {
    //   int err = snprintf(buf, BUF_LEN, "MALLOC: calloc(%zu, %zu) => (ptr=%p, size=%zu)\n", nmemb, size, 
    //                     get_pchunk_from_pdata(ptr), 
    //                     get_pchunk_from_pdata(ptr)->size);
    //   if (err < 0) {
    //     exit(1);
    //   }
    //   err = write(STDERR_FILENO, buf, buf_len);
    //   if (err == -1) {
    //     exit(1);
    //   }
    // }
    return NULL;
  } 

  ptr = memset(ptr, 0, actual_size);
  // if (getenv("DEBUG_MALLOC")) {
    // int err = snprintf(buf, BUF_LEN, "MALLOC: realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, 
                      // get_pchunk_from_pdata(ptr), 
                      // get_pchunk_from_pdata(ptr)->size);
    // if (err < 0) {
      // exit(1);
    // }
    // err = write(STDERR_FILENO, buf, buf_len);
    // if (err == -1) {
      // exit(1);
    // }
  // }
  return ptr;
}


void free(void* ptr) {
  if (ptr == NULL) {
    // sif (getenv("DEBUG_MALLOC")) {
    s  // int err = snprintf(buf, BUF_LEN, "MALLOC: free(%p)\n", ptr);
    s  // if (err < 0) {
    s    // exit(1);
    s  // }
    s  // err = write(STDERR_FILENO, buf, buf_len);
    s  // if (err == -1) {
    s    // exit(1);
    s  // }
    // s}
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
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: free(%p)\n", ptr);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return;
  }

  /* double free */
  if (!p_cur->in_use) {
    // if (getenv("DEBUG_MALLOC")) {
      // int err = snprintf(buf, BUF_LEN, "MALLOC: free(%p)\n", ptr);
      // if (err < 0) {
        // exit(1);
      // }
      // err = write(STDERR_FILENO, buf, buf_len);
      // if (err == -1) {
        // exit(1);
      // }
    // }
    return;
  }

  p_cur->in_use = false;

  /* merge with prev if cur is not the start of the heap */
  if (p_cur->prev != NULL && !p_cur->prev->in_use) {
    HeapChunk_t* prev_chunk = p_cur->prev;
    prev_chunk->next = p_cur->next;
    p_cur->next->prev = prev_chunk;
    prev_chunk->size = calc_user_chunk_size(prev_chunk);
  }

  /* merge with next if cur is not the last chunk in the heap */ 
  if (p_cur->next != NULL && !p_cur->next->in_use) {
    HeapChunk_t* next_chunk = p_cur->next;
    p_cur->next = next_chunk->next;
    // chunk is not 2nd to last of heap
    if (next_chunk->next != NULL) { 
      next_chunk->next->prev = p_cur;
    }
    p_cur->size = calc_user_chunk_size(p_cur);
  }

  // if (getenv("DEBUG_MALLOC")) {
    // int err = snprintf(buf, BUF_LEN, "MALLOC: free(%p)\n", ptr);
    // if (err < 0) {
      // exit(1);
    // }
    // err = write(STDERR_FILENO, buf, buf_len);
    // if (err == -1) {
      // exit(1);
    // }
  // }

  return;
}


static int init_heap() {
  intptr_t p_cur_end = (intptr_t)sbrk(0);
  intptr_t p_end_div_16 = GET_DIV16_VAL(p_cur_end);
  void* start_ptr = sbrk((p_end_div_16 - p_cur_end) + HEAP_INC_STEP);
  if (start_ptr == (void*)-1) {
    errno = ENOMEM;
    return HEAP_NOMEM_AVAIL;
  }
  intptr_t start_addr = p_end_div_16;
  heap_info.p_start = (HeapChunk_t*)start_addr;
  heap_info.p_last = (HeapChunk_t*)start_addr;
  heap_info.exists = true;
  heap_info.end_addr = (intptr_t) sbrk(0);

        
  heap_info.p_start->next = NULL;
  heap_info.p_start->prev = NULL;
  heap_info.p_start->in_use = false;
  heap_info.p_start->size = HEAP_INC_STEP - CHUNK_HEADER_SIZE;

  return INIT_HEAP_PASSED;
}


static HeapChunk_t* get_free_chunk(size_t size) {
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


/**
 * splits the chunk creating a new header after the req_size
 * will always be called when there is enough space for another header
*/
static void split_chunk(HeapChunk_t* chunk, size_t size) {
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


/**
 * request more mem from the OS
 * req_amt will be a multiple of 16
*/
static int ask_more_mem(size_t req_amt) {
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
  return ENOUGH_MEM; 

}


static bool space_for_another_chunk(HeapChunk_t* chunk, size_t req_size) {
  intptr_t new_chunk_end_addr = get_chunk_data_addr(chunk) + req_size + 
                                CHUNK_HEADER_SIZE;
  if (chunk->next == NULL) {
    return new_chunk_end_addr < heap_info.end_addr;
  }
  intptr_t next_chunk_addr = get_chunk_addr(chunk->next);
  return next_chunk_addr - new_chunk_end_addr >= MIN_CHUNK_SPACE;
}


static bool ptr_in_chunk(void* ptr, HeapChunk_t* chunk) {
  intptr_t ptr_addr = (intptr_t) ptr;
  intptr_t chunk_data_addr = get_chunk_data_addr(chunk);
  intptr_t chunk_end_addr = get_chunk_end_addr(chunk);
  return chunk_data_addr <= ptr_addr && ptr_addr < chunk_end_addr; 
}


static intptr_t get_chunk_end_addr(HeapChunk_t* chunk) {
  if (chunk->next == NULL) {
    return heap_info.end_addr;
  }
  return get_chunk_addr(chunk->next);
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


static HeapChunk_t* get_pchunk_from_pdata(void* ptr) {
  intptr_t ptr_addr = (intptr_t) ptr;
  ptr_addr -= CHUNK_HEADER_SIZE;
  return (HeapChunk_t*) ptr_addr;
}


