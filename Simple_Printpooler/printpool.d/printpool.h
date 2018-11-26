#include <pthread.h>

typedef struct {
  int aantal_printers;
  int aantal_taken;
  char **hosts;
  int *ports;
} printerpoolinfo_t;

typedef struct {
  void *printpool;
  char *host;
  int port; 
} printerinfo_t;

typedef struct {
  int connfd;
  struct sockaddr_in server;
} printer_conn_t;

typedef struct {
  char filenaam[20];
} printpool_taak_t;

typedef struct {
  int connfd;
  void *printpool;
} client_conn_info_t;

typedef struct printpool_t {
  pthread_mutex_t bezig;
  pthread_cond_t beschikbaar;
  pthread_t *printers;
  printpool_taak_t *taken;
  int aantal_printers;
  int aantal_taken;
  int volgende;
  int laatste;
  int huidig_taken;
}printpool_t;

printerpoolinfo_t *printerinfo_new(int aantal_printers, int aantal_taken, char **adresses, char **ports);

printpool_t *printpool_init(printerpoolinfo_t info);

void printpool_nieuwe_taak(printpool_t *pool, char filenaam[20]);

printer_conn_t printer_connection(void *printerinfo);