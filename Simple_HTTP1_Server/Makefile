all : server client

server : ./server.d/server.c
	     gcc -o ./server ./server.d/server.c -pthread

client : ./client.d/client.c
	     gcc ./client.d/client.c ./client.d/client.h -o ./client 

clean : 
	rm -f ./client ./server

rebuild:
	make clean
	make all
