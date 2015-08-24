
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int allocations =10000;

int
main( int argc, char **argv )
{

  printf("\n---- Running test7 ---\n");
  printf("Allocate many objects\n");
  char * ptrs[15410];
  int i;
  for (i = 0; i < allocations; i++ ) {
    char * p1 = (char *) malloc(100 );
    ptrs[i] = p1;
    *p1 = 100;
  }
  print_list();
  printf("----------- Free even ---------------\n");
  for(i = 0; i<allocations; i = i+2){
    free(ptrs[i]);
  }
  print_list();
  printf("------------ Free odd ----------------\n");
  for(i = 1; i<allocations; i = i+2){
    free(ptrs[i]);
  }
  print_list();

  printf("---- Allocate 100 blocks ----\n");
  for (i = 0; i < allocations; i++ ) {
    char * p1 = (char *) malloc(100 );
    ptrs[i] = p1;
    *p1 = 100;
  }
  print_list();
  exit( 0 );
}

