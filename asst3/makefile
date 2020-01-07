all: DUMBclient DUMBserver

DUMBclient:
	gcc -o DUMBclient DUMBclient.c
DUMBserver:
	gcc -o DUMBserver -lpthread DUMBserver.c
clean:
	rm DUMBclient DUMBserver
