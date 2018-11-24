#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "string.h"

#include "client.h"


#define TCP_PORT 8081
#define QUEUE_SIZE 10
#define POOL_SIZE 4

int main(int argc, char *argv[]){
    
    client_conn_t *client_conn  = client_init("localhost", TCP_PORT);
    printf("Server addr: %sConnfd: %d\n",  inet_ntoa(client_conn->server.sin_addr), client_conn->connfd);
    return 0;
}

client_conn_t *client_init(const char* server_addr, const int portno)
{
    int sockfd;
    struct hostent *server;
    struct in_addr **addr_list;
    client_conn_t *client_conn;

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
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port=htons(portno);
    if(inet_pton(AF_INET, inet_ntoa(*addr_list[0]), &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        exit(1);
    }
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }

    client_conn = malloc(sizeof(client_conn));
    client_conn->connfd = sockfd;
    client_conn->server = serv_addr;

    return client_conn;
}



