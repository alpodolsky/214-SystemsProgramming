#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main()
{
    clock_t start_t;
    clock_t end_t;
    int i, j, z;

    /***************** Test Case Work Load A *************************/

    /* 
        -> Malloc 1 byte and immediately free it amount:150 times 
        -> Workload A: malloc 1 byte and immediately free it amount:150 times 

    */

    double DurationOfA;
    double WorkLoadTestA;
    start_t = clock(); //Time for Work Load A started Running Here
    char *arrayOfA;
    
    for (i = 0; i < 100; i++) /*do each workload 100 times*/
    {   
        for (j = 0; j < 150; j++)
        {
            arrayOfA = (char *)malloc(1);
            if(arrayOfA == NULL){
                printf("Sorry, not enough memory to processed your request.");
            }
            //printf("Allocated: %d\n", sizeof(testA));
            free(arrayOfA);
        }

        //Time for Work Load A ends Running Here
        end_t = clock();
        DurationOfA = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestA += DurationOfA;
        //printf("%d: \n", i);
    }


    /***************** Work Load B *************************/

    /* B: malloc() 1 byte, store the pointer in an array 
        - do this 150 times.
        Once you've malloc()ed 50 byte chunks, 
        then free() the 50 1 byte pointers one by one.*/

    double WorkLoadTestB;
    double DurationOfB;
    
    char *ArraytestB[50];

    /*do each workload 100 times*/

    for (z = 0; z < 100; z ++)
    {

    start_t = clock();
    for (i = 0; i < 150; i++)
    {
        for (j = 0; j < 50; j++)
        {

            ArraytestB[j] = (char *)malloc(1);
            if (ArraytestB[j] == NULL)
            {
                printf("Sorry, not enough memory to processed your request.");
            }
        }

        for (j = 0; j < 50; j++)
        {
            free(ArraytestB[j]);
        }

        end_t = clock();
        DurationOfB = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestB += DurationOfB;
    }

}

        /***************** Test Case Work Load C *************************/

    /*
             Randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer
             -> Keep track of each operation so that you eventually malloc() 50 bytes, in total
             -> Keep track of each operation so that you eventually free() all pointers
        */

    double WorkLoadTestC; 
    double DurationOfC; 
    int numBytesAllocated = 0; 
    int mallocedBytes = 0, freedBytes = 0; 
    int randomByte;
    start_t = clock();
    char *testArryC[50];

    for (i = 0; i < 100; i++) //Do this 100 times 
    { 
        for (numBytesAllocated = 0; numBytesAllocated < 50; numBytesAllocated++) // Allocated 50 bytes
        {
            randomByte = rand() % 2; // Need at least a range of 2 numbers to generate a random byte in 

            if ((randomByte == 0 && mallocedBytes > 0) || mallocedBytes >= 50 - numBytesAllocated)
            {
                mallocedBytes--;
                free(testArryC[mallocedBytes]);
                //printf("j at: %d\n", j);
                freedBytes++;
            }

            else if (randomByte == 1 || mallocedBytes == 0)
            {
                testArryC[mallocedBytes] = (char *)malloc(1);
                 if (testArryC[mallocedBytes] == NULL)
                 {
                    printf("Sorry, not enough memory to processed your request.");
                 }
                mallocedBytes++;
            }

            //printf("numBytesAllocated at: %d\n", numBytesAllocated);
        }

        end_t = clock();
        DurationOfC = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestC += DurationOfC;
    }


/***************** Test Case Work Load D *************************/

    /*
         Randomly choose between a randomly-sized malloc() or free()ing a pointer 

         -> Keep track of each malloc so that all mallocs do not exceed your total memory capacity
         -> Keep track of each operation so that you eventually malloc() 50 times
         -> Keep track of each operation so that you eventually free() all pointers
         -> Choose a random allocation size between 1 and 64 bytes

        */

    double WorkLoadTestD;
    double DurationOfD;
    int mallocedBytesD = 0, freedBytesD = 0;
    int pickRandomSizeByte = 0;
    int numBytesAllocatedD, randomByteD;
    start_t = clock();
    char *testD[50];

    for (i = 0; i < 100; i++)
    {
        for (numBytesAllocatedD = 0; numBytesAllocatedD < 50; numBytesAllocatedD ++)
        {
            randomByteD = rand() % 2;
            pickRandomSizeByte = (rand() % 64); //A range of 64 bytes

            if ((randomByteD == 0 && mallocedBytesD > 0) || mallocedBytesD >= 50 - numBytesAllocatedD)
            {
                mallocedBytesD --;
                free(testD[mallocedBytesD]);
                freedBytesD ++;
                //printf("Free ended Here!\n");
            }

            else if (randomByteD == 1 || mallocedBytesD == 0)
            {
                testD[mallocedBytesD] = (char *)malloc(pickRandomSizeByte);
                if (testD[mallocedBytesD] == NULL)
                 {
                    printf("Sorry, not enough memory to processed your request.");
                 }
                mallocedBytesD++;
                //printf("%d, \n", mallocTrackerD);
            }
        }

        end_t = clock();
        DurationOfD = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestD += DurationOfD;
    }


    /***************** Test Case Work Load E *************************/

    /* 
    
    -> Check whether my malloc can allocate every type of bite size in power of 2
    -> Do 50 times for powers of 2 less than 128 for 150 times and make sure every pointer gets free()ed
    
    Why did we did this? Just to make test if out version malloc could handle every malloc size
    
    */
    
    double WorkLoadTestE;
    double DurationOfE;
    int BiteSize;
    start_t = clock(); // The time for this test starts here
    char *arrayOfE[64];

        for (i = 0; i < 100; i++){ //Do it 100 times
            for(j = 0; j <50; j ++){
                for(BiteSize = 0; BiteSize <= 64; BiteSize ++){  // Checking for memoy of 128 bytes
                BiteSize = BiteSize << 1; //For some reason is increasing in powers of 2 - 2
                arrayOfE[j] = (char *)malloc(BiteSize);
                //printf("Array at: %d\n", *arrayOfE[j]);
                //printf("j at: %d\n", BiteSize);  This is the part that I provided details on workload E in testplan.txt
                //Error check for malloc
                    if(arrayOfE[j] == NULL)
                    {
                        printf("Sorry, not enough memory to processed your request.");
                    }

                    free(arrayOfE[j]);
                    //printf("Freed\n");
                   // printf("j at: %d\n", j); //Just making sure that everything that got malloc()ed is free()ed
            }
        }

        end_t = clock(); // All loops have ran so stop the clock here
        DurationOfE = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestE += DurationOfE;
        //printf("i at : %d \n", i);
    }


    /***************** Test Case Work Load F *************************/
    /* 
    -> Malloc an array of 64 pointers
    -> As we were getting closer to the deadline time we basically created a case F that does the oppeosite of E
    -> For the sake of generality we ran this code 150 times as well
    */

    double WorkLoadTestF;
    double DurationOfF;
    int BiteSizeF;
    start_t = clock(); // The time for this test starts here
    char *arrayOfF[64];

        for (i = 0; i < 100; i++){ //Do it 100 times
            for(j = 0; j <50; j ++){
                for(BiteSizeF = 64; BiteSizeF > 0; BiteSizeF --){  // Checking for memoy of 128 bytes
                BiteSizeF = BiteSizeF >> 1; //For some reason is increasing in powers of 2 - 2
                arrayOfF[j] = (char *)malloc(BiteSizeF);
                //printf("Array at: %d\n", *arrayOfE[j]);
                //printf("j at: %d\n", BiteSize);  This is the part that I provided details on workload E in testplan.txt
                //Error check for malloc
                    if(arrayOfF[j] == NULL)
                    {
                        printf("Sorry, not enough memory to processed your request.");
                    }

                    free(arrayOfF[j]);
                    //printf("Freed\n");
                   // printf("j at: %d\n", j); //Just making sure that everything that got malloc()ed is free()ed
            }
        }

        end_t = clock(); // All loops have ran so stop the clock here
        DurationOfF = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        WorkLoadTestF += DurationOfF;
        //printf("i at : %d \n", i);
    }



    //By performing some testing I came to the conclusion that dividing workload by 10 provides me with the workLoad time in seconds....  
    //Never mind, some workloads need to be measured in microseconds
    printf("Workload A time for execution in microseconds: %.1f\n", ((WorkLoadTestA/100)*100000)); 
    printf("Workload B time for execution in microseconds: %.1f\n", ((WorkLoadTestB/100)*100000));  
    printf("Workload C time for execution in microseconds: %.1f\n", ((WorkLoadTestC/100)*100000));
    printf("Workload D time for execution in microseconds: %.1f\n", ((WorkLoadTestD/100)*100000));  
    printf("Workload E time for execution in microseconds: %.1f\n", ((WorkLoadTestE/100)*100000));
    printf("Workload F time for execution in microseconds: %.1f\n", ((WorkLoadTestF/100)*100000));
    printf("Total time for execution in microseconds: %.1f\n", 
    (((WorkLoadTestA + WorkLoadTestB + WorkLoadTestC + WorkLoadTestD + WorkLoadTestE + WorkLoadTestF)/100)*100000));
    return (0);

}
