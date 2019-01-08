#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "client.h"

#define FILENAAM_SIZE 80

static int global_connfd;

void INThandler(int signo){
    if(signo == SIGINT){
        fprintf(stderr, "\nCtrl-c Captured\n");
        close(global_connfd);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int portno;
     if (argc < 2) {
       fprintf(stderr,"usage %s hostname:port\n", argv[0]);
       exit(1);
    }
    char *hostname= strsep(&argv[1], ":");

    signal(SIGINT, INThandler);
    if(!client_init(hostname, atoi(argv[1]))){
        client_send_task();
        close(global_connfd);
    }
    return 0;
}

int *client_init(const char* server_addr, const int portno)
{
    int sockfd;
    struct hostent *server;
    struct in_addr **addr_list;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
    {
        fprintf(stderr, "Can't create Socket!\n");
        exit(1);
    }
    server = gethostbyname(server_addr);
    addr_list = (struct in_addr **) server->h_addr_list;

    if (server == NULL) {
        fprintf(stderr,"No such host!!\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    if(inet_pton(AF_INET, inet_ntoa(*addr_list[0]), &serv_addr.sin_addr) <= 0)
    {
        printf("\n Inet_pton error occured\n");
        exit(1);
    }
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }
    global_connfd = sockfd;
    return 0;
}

void client_send_task()
{
    char filenaam[FILENAAM_SIZE]; 
    int n; 
    while(1) { 
        memset(filenaam, '\0', FILENAAM_SIZE); 
        printf("Enter filenaam : "); 
        n = 0; 
        while ((filenaam[n++] = getchar()) != '\n');
        filenaam[strcspn(filenaam, "\n")] = '\0';
        write(global_connfd, filenaam, strlen(filenaam));
    }
}