all: server.out client.out clean
server.out : server.o strmap.o
	gcc -o server server.o strmap.o -pthread -std=c99
client.out : client.o strmap.o 
	gcc -o client client.o strmap.o -pthread -std=c99
strmap.o : strmap.c strmap.h
	gcc -c strmap.c
clean :
	rm -f server.o client.o strmap.o
