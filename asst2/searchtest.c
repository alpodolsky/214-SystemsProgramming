#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "multitest.h"

void main(int argc, char *argv){
    //ooooo adjustable size!
    int size = 2000;
    
    int *a= (int*)malloc((size)*sizeof(int));
    int i = 0;
    //just fills the array with values
    for(i=0;i<size;i++){
        a[i]=i+1;
    }
    /* if we change the size of the array variably then we should
        do the switch statement here and maybe wrap the code following?
        not sure about the wrap part
    */
    //amount of cases total, change as needed
    int testAmount = 2;
    //each case, can also be done with just this variable but cases need to be reversed, explained later
    int testcase = 0;
    int fakesize = 0;
    int divisor = 0;
    time_t start;
    time_t end;
    for(testcase = 0; testcase< testAmount; testcase++){
        switch(testcase){
            case 0:
                //size of the array we are going to use
                fakesize = 250;
                //how many each process or thread gets
                divisor = 50;
                break;
            /*
                if you add any other cases it has to be within these ranges basically
                so just change the bottom switch case to whatever the max number would be -1
                ie: you add 3 more cases, the bottom case should be case 4 and testAmount would be 5
            */
            default:
                fakesize = size;
                divisor = 250;
                break;
        }
            
        for(i = 0; i<(3*(fakesize/4));i++){
            int swap = 0;
            int firstindex=(rand())%fakesize;
            int secondindex = (rand())%fakesize;
            //ensures the swap pair is not swapping the same spot
            if(firstindex==secondindex){
                while(firstindex==secondindex){
                    secondindex=(rand())%fakesize;
                }
            }
            //super generic swap
            swap = a[firstindex];
            a[firstindex]=a[secondindex];
            a[secondindex]=swap;
        }
        //just to check it randomizes
        for(i = 0; i <fakesize;i++){
            if(i%10==0){
                printf("\n");
            }
            printf("%d\t", a[i]);
        }
    
        int numberToFind = rand()%fakesize;
        printf("\n%d is at index %d\n", a[numberToFind], numberToFind);
        start = time(0);
        int letsdo = dummysearch(a, fakesize, divisor, a[numberToFind]);
        end = time(0);
        float difference = start-end;
        printf("found %d at index %d took %f\n", a[numberToFind], letsdo, difference);
    } 
}