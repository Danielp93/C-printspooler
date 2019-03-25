#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "client.h"

#define BUFF_SIZE 255
#define REQ_SIZE 80

static int global_connfd;

int main(int argc, char *argv[])
{
    int portno;
     if (argc < 3) {
       fprintf(stderr,"usage %s <hostname:port> <file>\n", argv[0]);
       exit(1);
    }
    char *hostname = strsep(&argv[1], ":");

    if(!client_init(hostname, atoi(argv[1]))){
        client_send_request(argv[2]);
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

void client_send_request(char *filenaam)
{
    char buff[BUFF_SIZE];
    //Send request to server
    write(global_connfd, filenaam, strlen(filenaam));
    
    //clear buffer for reading from Socket
    memset(buff, '\0', BUFF_SIZE);
    //reading from socket/Writing to STDOUT
    int rc;
    while((rc = read(global_connfd, buff, BUFF_SIZE)) > 0)
    {
        if(write(STDOUT_FILENO, buff, rc) < 0)
        {
            write(STDOUT_FILENO, "Error Writing\n", strlen("Error Writing\n"));
        }
    }
    close(global_connfd);
    fflush(stdout);
}
