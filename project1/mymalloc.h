#ifndef MYMALLOC_H
#define MYMALLOC_H
#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)
#define memSize 4096
static char memory[memSize];

//meta data struct
struct Node {
    //smallest possible is 2 bytes(praise god if it actually is)
    //first bit is inUse, 3 empty, the last 12 are size(index is fromm 0 - 4095)
    unsigned short information;
};

//makes call to replace mymalloc:
    //-I think its done?
void *mymalloc(size_t size, char *filename, int line);
//makes call to replace myfree:
    //-still need
void myfree(void *p, char *filename, int line);

void set_used(struct Node* block);
void clear_used(struct Node* block);
void clear_size(struct Node* block);
void set_size(struct Node* block,unsigned short size);
int read_size(struct Node* block);
unsigned short read_used(struct Node* block);
void merge(struct Node* prev, struct Node* curr, struct Node* next);
void split(struct Node* block, int size, int currData);
#endif