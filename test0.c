
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"



int
main( int argc, char **argv )
{
  printf("\n---- Running test0 ---\n");

  //simple test that does nothing requiring malloc()
  int i = 2;
  int j = 3;
  int k = i + j;

  print_list();


  exit(0);
}
