
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int allocations = 10000;

int
main( int argc, char **argv )
{
  printf("\n---- Running test2 ---\n");
  
  //test performs many small allocations, up to 2MB
  int i;
  for ( i = 0; i < allocations; i++ ) {
    char * p1 = (char *) malloc(100 );
    *p1 = 100;
  }
  char * p2 = (char *) malloc(64);
  char * p3 = (char *) malloc(8);
  print_list();
  //printf(">>>> test2 passed\n\n");
  exit( 0 );
}

