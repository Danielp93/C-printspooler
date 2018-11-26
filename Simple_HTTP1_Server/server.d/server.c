#include <stdio.h>
#include <arpa/inet.h>//for htonl and sockaddr_in
#include <unistd.h>//for getopt,read,write,close
#include <stdlib.h>//for atoi
#include <string.h>//for memset and memcpy
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>//for errno
#include <signal.h>//for signal func
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define QUEUE_SIZE 30

typedef struct {
  int connfd;
  struct sockaddr_in server;
} client_conn_t;

static int stop = 0;

void * handle_client(void * connfd);

int main(int argc, char *argv[])
{ 
	int sockfd, connfd, portno, len;
    struct sockaddr_in servaddr, cli;
    struct hostent *server;

   if (argc < 2) {
         fprintf(stderr,"No port provided!\n");
         exit(1);
    }
    portno = atoi(argv[1]);
    
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		fprintf(stderr,"Can't open socket!\n"); 
		exit(1); 
	} 
	else{
		printf("Socket successfully created..\n"); 
	}
	
	bzero(&servaddr, sizeof(servaddr));
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(portno); 
	
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		fprintf(stderr,"Can't bind socket!\n"); 
		exit(1); 
	} 
	else
		printf("Socket successfully bound..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		fprintf(stderr,"Can't Listen!\n"); 
		exit(1); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

    pthread_t child;
    while(1){
        connfd = accept(sockfd, (struct sockaddr*)&cli ,&len); // accept awaiting request
        if(connfd!=-1){
			client_conn_t *client_conn_info = (client_conn_t *) malloc(sizeof(client_conn_info));
			client_conn_info->connfd = connfd;
			client_conn_info->server = cli;
            pthread_create(&child, NULL, handle_client, client_conn_info);
        }else{
            //sleep for 0.5 seconds
            usleep(500000);
        }
    }
    close(sockfd);
    return 0;
}

void * handle_client(void * client_conn_info)
{
    client_conn_t *connection = (client_conn_t *) client_conn_info;
    char request[40], fname[40];
	char    *filebuffer;
	long    numbytes;
	const char *errormessage = {"Malformed request:\nGET /<FILEPATH> HTTP/1.0\n"};
    while(1) {
		//clear out request
		memset(filebuffer, '\0', numbytes);
		memset(request, '\0', sizeof(request));
		// Read in client request
		if(read(connection->connfd, request, sizeof(request)) <= 0){
			fprintf(stderr, "\nA Client closed a connection.\n");
			fflush(stderr);
			close(connection->connfd);
            return 0;
		}
		fprintf(stdout, "[%s]%s\n", inet_ntoa(connection->server.sin_addr), request);
		char *reqpointer = strstr(request, " HTTP/1.0");
		if(reqpointer != NULL) {
			*reqpointer = '\0';
		}else{
			write(connection->connfd, errormessage, strlen(errormessage));
			continue;
		}
		if(strncmp(request, "GET /", strlen("GET /")) == 0) {
			strncpy( fname, &request[strlen("GET /")], strlen(request));	
		}else{
			write(connection->connfd, errormessage, strlen(errormessage));
			continue;
		}
		fname[sizeof(request)] = '\0';
		FILE *fp = fopen(fname, "rb");
		if(fp == NULL)
		{	
			write(connection->connfd, "Server Error: 404 File Not Found", sizeof(request));
			continue;
		}
		fseek(fp, 0L, SEEK_END);
		numbytes = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		filebuffer = (char*) malloc(numbytes * sizeof(char));if(filebuffer == NULL)
		{
			write(connection->connfd, "Can't read file", sizeof(request));
		}
		fread(filebuffer, sizeof(char), numbytes, fp);
		fclose(fp);
		write(connection->connfd, filebuffer, numbytes);
    }
    pthread_exit(NULL);
}