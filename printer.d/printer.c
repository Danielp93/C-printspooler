#include <stdio.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
 
int main(int argc, char *argv[])
{ 
	int sockfd, connfd, portno, len, n;
    struct sockaddr_in servaddr, cli;
    struct hostent *server;
    char buffer[40];

   if (argc < 2) {
         fprintf(stderr,"No port provided!\n");
         exit(1);
    }
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr, "Can't open socket!\n");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"No such host!\n");
        exit(1);
    }

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		fprintf(stderr,"No such host!\n"); 
		exit(1); 
	} 
	else
		printf("Socket successfully created..\n"); 
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

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
	if (connfd < 0) { 
		fprintf(stderr, "server acccept failed...\n"); 
		exit(1); 
	} 
	else
		printf("server acccept the client...\n"); 

    
	while (1)
    {
        memset(buffer, '0', sizeof(buffer));
        n = read(sockfd, buffer, 40);
        if (n < 0){
            fprintf(stderr, "Can't read from socket!\n");
		}

        int waittime = rand() % 10;
        printf("printing file %s, estimated wait time: %d seconds.\n", buffer);
        sleep(waittime);
        printf("done printing %s\n", buffer);
		
        n = write(sockfd, "Done", sizeof("done"));
        if (n < 0){
            fprintf(stderr, "Can't write to socket!\n");
		}
    }
	close(sockfd); 
}