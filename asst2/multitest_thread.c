#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "multitest.h"

struct threadInfo{
    int start;
    int end;
}threadInfo;

int toFind;
int *a;
int status;

void* singleTsearch(void* argse){
    struct threadInfo* args = (struct threadInfo*) argse;
    int begin = args->start;
    int last = args->end;

    while(begin<=last){
        if(a[begin]==toFind){
	    status = begin;//sets the global variable to whatever the index is
	    //makes sure to only get set once this way
	    pthread_exit(NULL);
	}
	begin++;
    }
    pthread_exit(NULL);
}

int search(int array[], int size, int breakSize, int target){
    a = array;//literally no idea why this works but I'm ok with it
    toFind = target;//makes target globally available for all threads
    int lastPieceSize;
    int numPieces = size/breakSize;
    if(size%breakSize!=0){
        lastPieceSize = size%breakSize;
        numPieces+=1;
    }
    else{
        lastPieceSize = breakSize;
    }
    pthread_t thread_id[numPieces];
    struct threadInfo tInfo[numPieces];
    int j;
    //makes each thread
    for(j = 0; j <numPieces;j++){
        //end is the only thing different between lastpiece and every other piece
        if(j==numPieces-1){
            tInfo[j].end = j*breakSize + lastPieceSize-1;
        }
        else{
	  tInfo[j].end = (j+1)*breakSize-1;
        }
        tInfo[j].start = j*breakSize;
	//when passed this way we can give the threads multiple variables used for its own computation
        pthread_create(&thread_id[j],NULL,singleTsearch, &tInfo[j]);
    }
    //waits for each thread to exit before continuing
    for(j = 0; j < numPieces; j++){
        pthread_join(thread_id[j], NULL);
    }
    return status;
}
