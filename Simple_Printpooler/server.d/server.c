#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include "server.h"

#define QUEUE_SIZE 30
#define FILE_SIZE 20

char *joblist[QUEUE_SIZE];
int current = 0;
pthread_mutex_t mutex;
printer_t *printers;
int pool_size;

int main(int argc, char *argv[]){

    if (argc < 3) {
         fprintf(stderr,"Usage ./server <port> [server1:port1 ... ServerX:portX]\n");
         exit(1);
    }

    int sockfd, connfd, i, len;
    pool_size = argc - 2;
    int tcp_port = atoi(argv[1]);
    struct sockaddr_in serv_addr, cli;
    pthread_t *printThreads;

    
    pthread_mutex_init(&mutex, NULL);
    //Allocate Memory for printers, Threads, and joblist
    printers = malloc(pool_size * sizeof(printer_t));
    printThreads = malloc(pool_size * sizeof(pthread_t));
    for(i = 0; i < QUEUE_SIZE; i++){
        joblist[i] = malloc(FILE_SIZE * sizeof(char));
    }
    
    //Initialize printers
    for(i = 0; i < pool_size; i++){
        char *hostname = strsep(&argv[i + 2], ":");
        printers[i].hostname = malloc(sizeof(&hostname));
        strncpy(printers[i].hostname, hostname, strlen(hostname));
        printers[i].port = atoi(argv[i + 2]);
        printers[i].busy = 0;
        printers[i].connection = printer_init(printers[i].hostname, printers[i].port);

        //init Printer Threads
        if(pthread_create(&printThreads[i], NULL, handle_printer, (void *) &printers[i])){
             fprintf(stderr, "Can't create printer thread %d\n", i);
             exit(1);
        }
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        printf("Error : Could not create socket\n");
        printf("Errno %d\n",errno);
        return -1;
    }   

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(tcp_port);
    len = sizeof(struct sockaddr);

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

    pthread_t child;
    while(1){
        if((connfd = accept(sockfd, (struct sockaddr*)&cli , &len)) == -1){
		 printf("Errno %d\n",errno);
	}
        if(connfd!=-1){
            if(child = pthread_create(&child, NULL, handle_client, (void *) &connfd) != 0){
                fprintf(stderr, "Error: Can't create thread!\n");
                exit(1);
            }
        }else{
            usleep(500000);
        }
    }
    close(sockfd);
    return 0;
}

void * handle_client(void * connfd)
{
    int connection = *(int *) connfd;
    char filenaam[FILE_SIZE];
    int numbytes;
    while(1) {
		//init filenaam and numbytes
		memset(filenaam, '\0', sizeof(filenaam)); 
		// Read in client numbers
		if((numbytes = read(connection, filenaam, sizeof(filenaam))) >= 0){
			if(numbytes > 0){
				pthread_mutex_lock(&mutex);
                		add_job(filenaam);
				pthread_mutex_unlock(&mutex);
			}else
			{
				fprintf(stderr, "Client Closed connection.\n");
				break;
			}
		}
    }
    pthread_exit(NULL);
}

void * handle_printer(void * printerconfig)
{
    printer_t *printer = (printer_t *) printerconfig;
    int i;
    char *filenaam;
    while(1){
            if(!printer->busy){
                printer->busy = 1;
    		pthread_mutex_lock(&mutex);
		filenaam = take_job();
		pthread_mutex_unlock(&mutex);
                if(filenaam > 0){
                    int waittime = rand() % 10;
                    write(printer->connection->connfd, filenaam, strlen(filenaam));
		            fprintf(stdout, "Printer[%s]: Printing %s|Duration: %d seconds.\n", printer->hostname, filenaam, waittime);
                    sleep(waittime);
                }
                printer->busy = 0;
            }
        }
    pthread_exit(NULL);
}

void add_job(char * filenaam)
{
    //clear out memory
    if(current < QUEUE_SIZE){
        memset(joblist[current], '\0', FILE_SIZE * sizeof(char));
        //Copy filenaam to joblist
        strncpy(joblist[current], filenaam, strlen(filenaam));
        current++;
    }else{
        fprintf(stdout, "File not added. Queue Full[%s]\n", filenaam);
    }
}

char* take_job()
{
    if(current > 0){
        current--;
        char * filenaam = malloc(strlen(joblist[current]) * sizeof(char));
        strncpy(filenaam, joblist[current], strlen(joblist[current]));
	return filenaam;
    }
    return 0;
}

printer_conn_t *printer_init(const char* server_addr, const int portno)
{
    int socketfd;
    struct hostent *server;
    struct in_addr **addr_list;
    printer_conn_t *printer_conn;

    if((socketfd = socket(AF_INET,SOCK_STREAM,0))<0)
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
    if(connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }

    printer_conn = malloc(sizeof(printer_conn));
    printer_conn->connfd = socketfd;
    printer_conn->server = serv_addr;
    return printer_conn;
}
