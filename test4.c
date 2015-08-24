

#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"

int allocations = 50000;

int
main( int argc, char **argv )
{
  printf("\n---- Running test4 ---\n");
  
  //test performs many small allocations, beyond 2MB.
  //this means more blocks must be requested from the OS.
  int i;
  for ( i = 0; i < allocations; i++ ) {
    char * p1 = (char *) malloc(100 );
    *p1 = 100;
  }
  print_list();
  //printf(">>>> test2 passed\n\n");
  exit( 0 );
}

