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

#include "../printpool.d/printpool.h"


#define TCP_PORT  9000
#define QUEUE_SIZE 30
#define POOL_SIZE 3

static bool stop = 0;

void * handle_client(void * pool);

int main(int argc, char *argv[]){

    int sockfd, connfd, i;
    int tcp_port = TCP_PORT;
    printerpoolinfo_t info;
    printpool_t *pool;
    char hosts[POOL_SIZE][20] = {"localhost", "localhost", "localhost"};
    int ports[POOL_SIZE] = {8080, 8081, 8082};

    struct sockaddr_in serv_addr;
    char opt;

    if((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))== -1)
    {
        printf("Error : Could not create socket\n");
        printf("Errno %d\n",errno);
        return -1;
    }   

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(tcp_port);

    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1){
        fprintf(stderr, "Socket binding failed\n");
        if(errno == EADDRINUSE)
            fprintf(stderr,"Another socket is already listening on the same port\n");
        return -1;
    }

    if(listen(sockfd, QUEUE_SIZE) == -1){
        printf("Error:Failed to listen\n");
        printf("Errno %d\n",errno);
        if(errno == EADDRINUSE)
            printf("Another socket is already listening on the same port\n");
        return -1;
    }

    printf("Listening on TCP port %d\n",tcp_port);


    memset(&info, '0', sizeof(printerpoolinfo_t));
    info.aantal_printers = POOL_SIZE;
    info.aantal_taken = QUEUE_SIZE;
    info.hosts = malloc(POOL_SIZE * sizeof(char *));
    info.ports = malloc(POOL_SIZE * sizeof(int)); 
    for(i = 0; i < POOL_SIZE; i++)
    {
        info.hosts[i] = malloc(sizeof(hosts[i]));
        strncpy(info.hosts[i], hosts[i], strlen(hosts[i]));
        info.ports[i] = ports[i];
    }

    pool=printpool_init(info);
    pthread_t child;
    while(1){
        connfd = accept(sockfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        if(connfd!=-1){
            client_conn_info_t *client_info = malloc(sizeof(client_conn_info_t));
            client_info->printpool = malloc(sizeof(pool));
            client_info->connfd = connfd;
            client_info->printpool = pool;

            pthread_create(&child, NULL, handle_client, client_info);
        }else{
            //sleep for 0.5 seconds
            usleep(500000);
        }
    }
    close(sockfd);
    return 0;
}

void * handle_client(void * client_info)
{
    client_conn_info_t *info = (client_conn_info_t *) client_info;

    char filenaam[20];
    while(1) {
		//clear out filenaam
		bzero(filenaam, sizeof(filenaam)); 
		// Read in client numbers
		if(read(info->connfd, filenaam, sizeof(filenaam)) <= 0){
			fprintf(stderr, "\nA Client closed a connection.\n");
			close(info->connfd);
            return 0;
		}
        printpool_nieuwe_taak((printpool_t *) info->printpool, filenaam);

		fprintf(stdout, "\nNew Task %s added.\n", filenaam);
    }
}