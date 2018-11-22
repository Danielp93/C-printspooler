#include <stdio.h>
#include <arpa/inet.h>//for htonl and sockaddr_in
#include <unistd.h>//for getopt,read,write,close
#include <stdlib.h>//for atoi
#include <string.h>//for memset and memcpy
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>//for errno
#include <signal.h>//for signal func
#include "server_comm.h"
#include "threadpool.h"
#include <stdbool.h>

#define TCP_PORT  8080
#define QUEUE_SIZE 10
#define POOL_SIZE 3

bool stop;
const int queue_size=15;


int main(int argc, char *argv[]){
    int listenfd, connfd;
    int num_threads=POOL_SIZE;
    int tcp_port = TCP_PORT;

    struct sockaddr_in serv_addr;
    char opt;

    if((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))== -1)
    {
        printf("Error : Could not create socket\n");
        printf("Errno %d\n",errno);
        return -1;
    }   

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(tcp_port);

    if(bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1){
        fprintf(stderr, "Error:Bindint with port # %d failed\n",tcp_port);
        fprintf(stderr, "Errno %d\n",errno);
        if(errno == EADDRINUSE)
            printf("Another socket is already listening on the same port\n");
        return -1;
    }

    if(listen(listenfd, QUEUE_SIZE) == -1){
        printf("Error:Failed to listen\n");
        printf("Errno %d\n",errno);
        if(errno == EADDRINUSE)
            printf("Another socket is already listening on the same port\n");
        return -1;
    }

    printf("Lintning on TCP port %d\n",tcp_port);

    threadpool_t *tp=threadpool_create(num_threads,queue_size,0);
    printf("Thread pool size %d\n",tp->thread_count);
    
    while(1){
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        if(connfd!=-1){
            threadpool_add(tp,server_run,connfd,0);

        }else{
            //sleep for 0.5 seconds
            usleep(500000);
        }
    }
    
    close(listenfd);
    return 0;
}