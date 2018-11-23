#include <pthread.h>

typedef struct {
  int aantal_printers;
  int aantal_taken;
  char **hostnames;
} printerinfo_t;

typedef struct {
    char filenaam[10];
} printpool_taak;

typedef struct printpool_t {
  pthread_mutex_t bezig;
  pthread_cond_t beschikbaar;
  pthread_t *printers;
  printpool_taak *taken;
  int aantal_printers;
  int aantal_taken;
  int volgende;
  int laatste;
  int huidig_taken;
}printpool_t;

printerinfo_t *printerinfo_new(int aantal_printers, int aantal_taken, char **adresses, char **ports);

printpool_t *printpool_init(printerinfo_t info);

void printpool_nieuwe_taak(printpool_t *pool, char filenaam[10]);