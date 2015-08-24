#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "MyMalloc.h"

#define maxallocs 10


void *allocationThread(void *none){
    int i,j;
    for(i=0;i<maxallocs;i++){
        char * p1 = (char *) malloc(10);
        *p1 = 100;
    }
}

int main(){
    printf("\n---- Running test6 ---\n");
    int numThreads = 5;
    malloc(20);

    pthread_t threads[numThreads];
    int i;
    for(i=0;i<numThreads;i++){
        pthread_create(&threads[i],NULL,allocationThread,NULL);
    }

    for(i=0;i<numThreads;i++){
        pthread_join(threads[i],NULL);
    }

    print_list();
    exit(0);
}
