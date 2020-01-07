#include <stdio.h>
#include <stdlib.h>
#include "multitest.h"

int singlePSearch(int array[], int start, int end , int target){
	while (start <= end){
		if (array[start] == target){
			return start;
		}
		start++;
	}
	return -1;
}

//start with simple single process search
int search(int array[], int size, int breakSize, int target){
	int exitStatus = -1;
	int start = 0;
	int end = size-1;
	int forkVal; //to determine if parent or child
	int origPid = getpid(); //need pid of original process
	int processNum = 0; //will need in order to translate index
	int lastPieceSize;

	
	int numPieces = size/breakSize; //get the number of sections array must be broken up into
	if (size % breakSize != 0){
		lastPieceSize = size % breakSize;
		printf("remainder is: '%d'\n", lastPieceSize);
		numPieces += 1;
	}
	else{
		lastPieceSize = breakSize;
	}

	int* pidStorage = (int*)malloc(numPieces * sizeof(int)); //to store the pids of the child processes
	int* newArray = (int*)malloc(breakSize); //for "pieces" of original array to give to child processes
	
	//only want to fork off of original process
	if (origPid == getpid()){ //PROBLEM IS IN THIS CURLY BRACE SECTION SOMEWHERE
		int i = 0;
		int j = 0;
		end = (start + breakSize)-1;
		while(j < breakSize){
			newArray[j] = array[start];
			j++;
			start++;
		}
		forkVal = fork();
		pidStorage[processNum] = forkVal;
		while (i < numPieces-2){ //-2 to account for last piece & original process
			if (origPid == getpid()){
				start = (end+1);
				end += breakSize;
				processNum++;
				//create new array
				int j = 0;
				while (j < breakSize){
					newArray[j] = array[start];
					j++;
					start++;
				}


				printf("process number is '%d'\n", processNum);
				forkVal = fork();
				pidStorage[processNum] = forkVal;
			}
			i++;
		}
		
	 //THIS TECHNICALLY WAS SUPPOSED TO HANDLE "REMAINDER" SECTIONS
		if (origPid == getpid() && numPieces > 1){
			start = (end+1);
			end += lastPieceSize;
			processNum++;

			int j = 0;
			while (j < lastPieceSize){
				newArray[j] = array[start];
				j++;
				start++;
			}
			breakSize = lastPieceSize;
			forkVal = fork();
			pidStorage[processNum] = forkVal;
		}
		
	
	}
	
	//want all processes to run the code below. We should fork right above here... I think
	if (forkVal == 0){
		exitStatus = singlePSearch(newArray, 0, breakSize-1, target); //want result of searching to be exit status
	}

	if (forkVal > 0){ //is parent process
		int status;
		int pToWaitFor = 0;
		while (pToWaitFor < numPieces){
			waitpid(pidStorage[pToWaitFor], &status, 0);
			printf("waited for: '%d' \n", pidStorage[pToWaitFor]);
			if (WEXITSTATUS(status) != -1 && WEXITSTATUS(status) != 255){
				if (pToWaitFor > 0){ //not in first "piece" of array -- has different indexes
				//translate index here
					int origIndex = WEXITSTATUS(status);
					int translatedIndex = origIndex + (pToWaitFor * breakSize);
					return translatedIndex;
				}
				else{
					printf("exit status of '%d' was: '%d' \n", pidStorage[pToWaitFor], WEXITSTATUS(status));
					return WEXITSTATUS(status);
				}	
			}
			pToWaitFor++;
		}
		
		printf("No process found the target\n");
		return -1;
	}
	
	printf("Process '%d' Exiting with status: '%d'\n", getpid(), exitStatus);
	exit(exitStatus);
}
