#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "mymalloc.h"

//1 means initializzed, 0 means first time
int initialized=0;
int dataUsed=0;

struct Node* firstblock=NULL;
struct Node* lastblock=NULL;


void *mymalloc(size_t size, char *filename, int line){
    int currData = 0;
    //request 0 or negative
    //printf("%d data used\n", dataUsed);
    if(size<=0){
        fprintf(stderr,"Error: Cannot allocate negative memory, File: %s, Line: %d", filename, line);
        return NULL;
    }//request is bigger than memory
    if((size +sizeof(struct Node)>=4096)||size>=4096){
        fprintf(stderr,"Storage Error: Data too large, File: %s, Line: %d",filename, line);
        return NULL;
    }//request is bigger than memory left
    if(size+dataUsed+sizeof(struct Node) >= 4096){
        fprintf(stderr,"Storage Error: system will run out of space, File: %s, Line: %d", filename, line);
        return NULL;
    }//first malloc
    if(initialized == 0){
        //fills the firstblock with data
        firstblock=(void*)(&memory[0]);
        clear_size(firstblock);
        set_size(firstblock, 4094);
        split(firstblock, size, currData);
        set_used(firstblock);
        dataUsed =((2*sizeof(struct Node))+size);//accounts for first and last block
        //makes the following block as well, probably dont need this
        clear_used(lastblock);
        initialized = 1;
        return (void*)(firstblock+1);
    }
    if(initialized == 1){
        struct Node* ptr = firstblock;
        
        int currSize= 0;
        int currData=0;
        //printf("\n");
        //printf("\n");
        //printf("%d in use\n", read_used(ptr));
        //printf("%p firstblock, %p lastblock\n", firstblock, lastblock);
        while(ptr!=lastblock){
            //printf("%d in use %p\n", read_used(ptr), ptr);
            //printf("%d currdata\n", currData);
            if(read_used(ptr)==0){
                break;
            }
            //current block is not in use and does not have size
            //current block is in use
            //printf("%p ptr point\n", ptr);
            currData+=(read_size(ptr)+sizeof(struct Node));
            if(currData>=4094){
                break;
            }
            ptr=(void*)(&memory[currData]);
            //printf("%p ptr point\n", ptr);
            
        }//on last block
            //last block can be in use
       // printf("%d read used, %d read size, %p pointer\n", read_used(ptr), read_size(ptr), ptr);
        if(read_used(ptr)==1&&ptr==lastblock){
            fprintf(stderr,"Malloc Error: not enough space found first, File: %s, Line: %d\n", filename, line);
            return NULL;
        }//not in use and not enough space- error
        
        if(read_used(ptr)==0&&read_size(ptr)<size){
            fprintf(stderr,"Malloc Error: not enough space found, File: %s, Line: %d\n", filename, line);
            return NULL;
        }//not in use and enough space, no need to split cause block is perfect
        if(read_used(ptr)== 0 &&read_size(ptr)==size){
            //printf("checking1 %d currdata\n", currData);
            //printf("%d size of curr block, %d next bloc, %p location\n",read_size(ptr),read_size((void*)(&memory[currData])), ptr);
            set_used(ptr);
            dataUsed+= read_size(ptr);
            return (void*)(ptr+1);
        }
        if(read_used(ptr)==0&&(read_size(ptr)>=(size+sizeof(struct Node)))){
            //printf("checking2 %d currdata\n", currData);
            //printf("%d size of curr block, %d next block\n",read_size(ptr),read_size((void*)(&memory[currData+read_size(ptr)])));
            //printf("checking2 %p ptr\n", ptr+1);
            split(ptr, size, currData);
            set_used(ptr);
            dataUsed+=size;
            //printf("checking2 %p ptr\n", ptr+1);
            return (void*)(ptr+1);
        }
    }
}
void myfree(void *p, char *filename, int line){
    //yoo to be honest I have no clue how this should be handled but:
        //Obviously its an error but im gonna assume if it hasn't been init
        //then it wouldn't flip/count as the first call to malloc
    //printf("in free\n");
    struct Node* ptr, *prev, *next;
    if(initialized ==0){
        fprintf(stderr,"Why are you freeing the first time?\n");
        return;
    }
    if(initialized == 1){
        int currDat = 0;
        ptr = (void*)(&memory[0]);
        prev = NULL;
        currDat= (read_size(ptr)+sizeof(struct Node));
        //printf("%d currDat %d size of block\n",currDat, read_size(ptr));
        next = (void*)(&memory[currDat]);
        //printf("p address %p, ptr address %p, next address %p\n", p, ptr+1, next+1);
        while((ptr+1)!=p||ptr!=lastblock){
            currDat+=(read_size(next)+sizeof(struct Node));
            if(currDat>=4096){//end of data
                next=0;
                break;
            }
            //generic ll movement
            prev = ptr;
            ptr = next;
            //printf("%d currDat %d size of block\n",currDat, read_size(ptr));
            //printf("p address %p, ptr address %p, next address %p, last block %p\n", p, ptr+1, next+1, lastblock);
            //totally nongeneric way to increment a ptr

            next =(void*)(&memory[currDat]);
        }
        if(ptr+1 == p){
            //printf("found match2\n");
            //printf("p address %p, ptr address %p, next address %p, ptr address %p\n", p, ptr+1, next, prev);
            clear_used(ptr);
            //printf("%d\n",dataUsed);
            dataUsed-=(read_size(ptr));
            //printf("%d\n",dataUsed);
            if((prev==NULL||read_used(prev)==1)&&read_used(next)==1){
                //printf("please\t");
                return;
            }
            merge(prev, ptr, next);
            return;
        }
    }
}
void clear_used(struct Node* block){
    block->information &=~(1<<15);
}
void set_used(struct Node* block){
    block->information|=(1<<15);
}
unsigned short read_used(struct Node* block){
    return (block->information>>15)&1;
}
int read_size(struct Node* block){ 
    return ((block->information)&=~(1<<15));
}
void set_size(struct Node* block,unsigned short size){
    block->information|=size;
}
void clear_size(struct Node* block){
    block->information&=0;
}
//merge helper, maybe change this to return a pointer?
void merge(struct Node* prev, struct Node* curr, struct Node* next){
    //three cases
    //printf("%p prev location, %p curr location, %p next loocation\n", prev, curr, next);
        //first: all 3 are free
    //printf("inmemrge\n");
            //prev does not exist or is in use
            //known working case
    if(prev==0&&read_used(next)==0){
        //printf("your second merge2 case\n");
        //give curr the size of next + its struct size
        //printf("%d size of curr block, %d next block\n",read_size(curr),read_size(next));
        int newSize =read_size(next)+sizeof(struct Node)+read_size(curr);
        //printf("%d nesize\n", newSize);
        clear_size(curr);//SERIOUSLY THIS IS JUST STUPID
        set_size(curr,newSize);
        //printf("%d dataUsed, %d size of curr block\n", dataUsed, read_size(curr));
        dataUsed-=sizeof(struct Node);
        //printf("%d dataUsed\n", dataUsed);
        return;
    }
    if(next==0&&read_used(prev)==0){
        //printf("your third merge2 case\n");
        //give prev the size of curr + its struct size
        int newSize =read_size(prev)+sizeof(struct Node)+read_size(curr);
        clear_size(prev);
        set_size(prev,newSize);
        clear_used(prev);
        dataUsed-=sizeof(struct Node);
        return;
    }
    if(read_used(prev)==1&&read_used(next)==0){
        //printf("your second merge case\n");
        //give curr the size of next + its struct size
        //printf("%d size of curr block, %d next block\n",read_size(curr),read_size(next));
        int newSize =read_size(next)+sizeof(struct Node)+read_size(curr);
        //printf("%d nesize\n", newSize);
        clear_size(curr);//SERIOUSLY THIS IS JUST STUPID
        set_size(curr,newSize);
        //printf("%d dataUsed, %d size of curr block\n", dataUsed, read_size(curr));
        dataUsed-=sizeof(struct Node);
        //printf("%d dataUsed\n", dataUsed);
        return;
    }
        //case 2 again but for next
    if(read_used(next)==1&&read_used(prev)==0){
        //printf("your third merge case\n");
        //give prev the size of curr + its struct size
        int newSize =read_size(prev)+sizeof(struct Node)+read_size(curr);
        clear_size(prev);
        set_size(prev,newSize);
        dataUsed-=sizeof(struct Node);
        return;
    }
    
    if(read_used(prev)==0&&read_used(next)==0){
        //printf("your first merge case\n");
        //gives prev the size of all 3 + 2 of the structs
        int newSize =read_size(prev)+(2*sizeof(struct Node))+read_size(curr)+read_size(next);
        clear_size(prev);
        set_size(prev,newSize);
        //printf("%d size of new", read_size(prev));
        //printf("\t%p firstblock, %p lastblock\n", firstblock, lastblock);
        dataUsed-=(2*sizeof(struct Node));
        //need to some how give back ptr to prev
        return;
    }
    //printf("did nothing\n");
}
//keep in mind if you use split it changes the block to in use, probably wont be used for free
void split(struct Node* block, int size, int currData){
    //find size of next block = blockDataSize - requested size - sizeofstruct
    int diffSize = read_size(block)-size-sizeof(struct Node);
    //printf("%d sizdiff\t", diffSize);
    //need to set the size of block to requested
    clear_size(block);
    set_size(block,size);
    set_used(block);
    //printf("%d block size %d currData\t", read_size(block), currData);
    //set_used(block);//and change beingUsed, maybe do this manually after a split
    //set next to be after data pointer
    //printf("%p block, %p lastblock", block, lastblock);
    if(block==lastblock||initialized==0){
        lastblock =  (void*)(&memory[currData+read_size(block)+sizeof(struct Node)]);
        clear_size(lastblock);
        set_size(lastblock, diffSize);
        //printf("%d block size of new last block, %p its location\n", read_size(lastblock), lastblock);
        clear_used(lastblock);
        return;
    }

    struct Node* next =(void*)(&memory[currData+read_size(block)]);
    dataUsed+=sizeof(struct Node);
    clear_size(next);
    set_size(next,diffSize);
    clear_used(next);
    //printf("%d block size\n", read_size(next));
    return;
   
}
