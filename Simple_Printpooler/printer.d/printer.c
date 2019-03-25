#include <stdio.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
 
void printing(int connfd);

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
		printf("Printer listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
	if (connfd < 0) { 
		fprintf(stderr, "server acccept failed...\n"); 
		exit(1);
	} 
	else{ 
		printf("server acccepted the client...\n");
	}

	printing(connfd);
	
    close(sockfd); 
}

void printing(int sockfd) 
{ 
	char buff[80]; 
	int n; 
	while(1) {
		//clear out buff
		memset(buff, '\0', sizeof(buff)); 
		if(read(sockfd, buff, sizeof(buff)) <= 0){
			fprintf(stderr, "Closing down socket!");
			exit(1);
		}
		fprintf(stdout, "Printing[%s]\n", buff);
	} 
} 