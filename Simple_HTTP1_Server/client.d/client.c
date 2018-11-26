#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "string.h"

#include "client.h"


#define TCP_PORT 9000

static int running = 1;

void INThandler(int signo){
    if(signo == SIGINT){
        fprintf(stderr, "\nCTRL-C CAPTURED\n");
        running = 0;
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
    client_send_task(client_conn);
    close(client_conn->connfd);
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

    client_conn = malloc(sizeof(client_conn));
    client_conn->connfd = sockfd;
    client_conn->server = serv_addr;

    return client_conn;
}

void client_send_task(client_conn_t *client_conn)
{
    char request[40];
    int n, inc_length, offset, total_length =0;
    char *response = NULL;
    while(running) {
        //Zero out request buffer and variables for new request
        memset(request, 0, sizeof(request));  
        n = 0;
        //Input Request -> GET /<FILE> HTML/1.0
        while ((request[n++] = getchar()) != '\n');
        //Clearance for only \n character
        if(strlen(request) <= 1){
            continue;
        }
        //strip newline from request
        request[strcspn(request, "\n")] = 0;
        //Send request to server
        write(client_conn->connfd, request, strlen(request));
        //Wait for 0.5 secs so server can respond
        usleep(500000);
        while(1){
            
            //Check size of incomming message, if nothing comming, print message.
            ioctl(client_conn->connfd, FIONREAD, inc_length);
            fprintf(stdout, "%d\n", inc_length);
            if(inc_length != 0){
                total_length += inc_length;
                //Allocate memory for new message block
                response = realloc(response, total_length);
                if (NULL == response)
                {
                    perror("realloc");
                    abort();
                }
                offset = total_length - inc_length;
                //Write new message block to end of old message
                read(client_conn->connfd, response + offset, inc_length);
            }
            else{
                fprintf(stdout, "%s\n", response);
                fflush(stdout);
                memset(response, 0, sizeof(response));
                break;
            }
        }
    }
}