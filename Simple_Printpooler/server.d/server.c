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

#define QUEUE_SIZE 30

static bool stop = 0;

void * handle_client(void * pool);

int main(int argc, char *argv[]){

    int sockfd, connfd, i;
    printerpoolinfo_t info;
    printpool_t *pool;

    int pool_size = argc - 2;
    int tcp_port = atoi(argv[1]);

    char **hosts = malloc(sizeof(char *) * (argc - 1));
    int *ports = malloc(sizeof(int) * (argc - 1));

    for(i = 0; i < pool_size; i++){
        char *hostname= strsep(&argv[i + 2], ":");
        hosts[i] = malloc(sizeof(&hostname));
        strncpy(hosts[i], hostname, strlen(hostname));
        ports[i] = atoi(argv[i + 2]);
    }

    for(i = 0; i < pool_size; i++){
        printf("%s %d\n", hosts[i], ports[i]);
    }

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
    info.aantal_printers = pool_size;
    info.aantal_taken = QUEUE_SIZE;
    info.hosts = malloc(pool_size * sizeof(char *));
    info.ports = malloc(pool_size * sizeof(int)); 
    for(i = 0; i < pool_size; i++)
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