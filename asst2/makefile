all: multitest_proc multitest_thread

multitest_proc: searchtest.c multitest_proc.c multitest.h
	gcc -c multitest_proc.c
	gcc -o multitest_proc searchtest.c multitest_proc.o

multitest_thread: searchtest.c multitest_thread.c multitest.h
	gcc -c multitest_thread.c 
	gcc -o multitest_thread -lpthread searchtest.c multitest_thread.o

clean:
	rm multitest_proc multitest_proc.o multitest_thread multitest_thread.o
