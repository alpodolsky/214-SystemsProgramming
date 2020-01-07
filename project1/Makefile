all: memgrind

memgrind: memgrind.c mymalloc.c mymalloc.h
	gcc -c mymalloc.c
	gcc -g -o memgrind memgrind.c mymalloc.o

clean:
	rm memgrind mymalloc.o