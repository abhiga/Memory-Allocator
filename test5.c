
#include <stdlib.h>
#include <stdio.h>
#include "MyMalloc.h"



int
main( int argc, char **argv )
{

  printf("\n---- running test5 ---\n");
  //fence posts
  int * mem1 = (int *) malloc( 1048576 );
  *mem1 = 10;

  int * mem2 = (int *) malloc( 1048528 );
  *mem2 = 11;
  
  printf("NO FREES:\n");
  print_list();
  free(mem2);
  printf("FREE RIGHT:\n");
  print_list();
  free(mem1);
  printf("FREE LEFT:\n");
  print_list();


  exit(0);
}
