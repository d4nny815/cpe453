#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lib.h"
#include "malloc.h"

#define HOWMANY 5

int main(int argc, char *argv[]){
  unsigned char *val;
  int i;
  void *hunk[HOWMANY];

  for(i=0;i<HOWMANY;i++ ) {
    size_t size = i * 500;

    printf("Allocating a region of size %d...", (int)size);
    val = malloc (size);
    if ( !val && size )
      printf("FAILED.\n");
    else {
      fill(val,size,i);
      if ( !check(val,size,i) )
        printf("ok.\n");
      else
        printf("FAILED.\n");
    }
    hunk[i]=val;
  }
  print_heap();
  /* check that they all survived */
  for(i=0;i<HOWMANY;i++ ) {
    size_t size = i * 500;

    printf("Verifying contents of region of size %d...", (int)size);
    val=hunk[i];
    if ( !val && size )
      printf("FAILED.\n");
    else {
      if ( !check(val,size,i) )
        printf("ok.\n");
      else
        printf("FAILED.\n");
    }
  }

  exit(0);
}
