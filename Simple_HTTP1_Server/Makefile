all : server client

server : ./server.d/server.c
	     gcc -o ./server ./server.d/server.c -pthread

client : ./client.d/client.c
	     gcc -o ./client ./client.d/client.c

clean : 
	rm -f ./client ./server
