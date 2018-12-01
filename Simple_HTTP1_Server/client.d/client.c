#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "client.h"

#define BUFF_SIZE 255
#define REQ_SIZE 80

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
    client_conn_t *client_conn  = client_init(hostname, atoi(argv[1]));
    client_send_request();
    close(global_connfd);
    free(client_conn);
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
    global_connfd = sockfd;
    return client_conn;
}

void client_send_request()
{
    char buff[BUFF_SIZE];
    int n, numbytes;
    while(1) {
        n = 0;
        //Input Request -> GET /<FILE>
        while ((buff[n++] = getchar()) != '\n');
        //Clearance for only \n character
        if(strlen(buff) <= 1){
            continue;
        }
        //Send request to server
        write(global_connfd, buff, strlen(buff));
        memset(buff, '\0', BUFF_SIZE);
        while((numbytes = read(global_connfd, buff, BUFF_SIZE)) > 0)
		{
            if(numbytes < BUFF_SIZE){
			    write(1, buff, numbytes);
                memset(buff, '\0', BUFF_SIZE);
                break;
            }
		    write(1, buff, BUFF_SIZE);
		}
        fprintf(stdout, "\n");
    }
}