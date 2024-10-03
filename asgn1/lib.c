#include<stdlib.h>
#include<stdio.h>

void fill(unsigned char *s, size_t size, int start) {
  int i;
  for(i=0; i< size; i++)
    s[i]=start++;
}

int check(unsigned char *s, size_t size, int start){
  int i,err=0;
  for(i=0; i < size; i++,start++)
    err += (s[i] != (start&0xff));

  return err;
}

void *getbuff(size_t size){
  unsigned char *p;
  p = malloc(size);
  if ( p )
    printf("Calling malloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  fill(p,size,0);
  printf("Successfully used the space.\n");

  if ( check(p,size,0) )
    printf("Some values didn't match in region %p.\n",p);

  return p;
}
