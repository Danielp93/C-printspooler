#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFF_SIZE 255

typedef struct {
  int connfd;
  struct sockaddr_in server;
} client_conn_t;

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
	memset(&servaddr, '\0', sizeof(servaddr));
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
			fprintf(stderr, "Client[%s] connected.\n", inet_ntoa(client_conn_info->server.sin_addr));
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
	char buff[BUFF_SIZE], fname[30];

	int numbytes;
	memset(buff, '\0', BUFF_SIZE * sizeof(char));
	if((numbytes = read(connection->connfd, buff, BUFF_SIZE)) > 0){
		fprintf(stdout, "[%s]%s\n", inet_ntoa(connection->server.sin_addr), buff);
	}
	buff[strcspn(buff, "\n")] = '\0';
	if(strncmp(buff, "GET /", strlen("GET /")) == 0) {
		strncpy( fname, &buff[strlen("GET /")], strlen(buff));	
	}else{
		strncpy(fname, buff, strlen(buff));
	}
	int fd = open(fname, O_RDONLY, 0);
	if(fd == -1)
	{	
		write(connection->connfd, "Server Error: 404 File Not Found\n", strlen("Server Error: 404 File Not Found\n"));
	}
	memset(buff, '\0', BUFF_SIZE);
	while((numbytes = read(fd, buff, BUFF_SIZE)) > 0)
	{
		if(write(connection->connfd, buff, numbytes) != numbytes)
		{
			fprintf(stderr, "Error Sending Packet[%d]\n", numbytes);
		}
	}
	close(fd);
	close(connection->connfd);
	free(connection);
    pthread_exit(NULL);
}