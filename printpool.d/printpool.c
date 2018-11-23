#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../printpool.d/printpool.h"

static void *printer(void *printerinfo);

printpool_t *printpool_init(printerpoolinfo_t info)
{
    printpool_t *pool;
    printerinfo_t **printerinfos;
    int i;

    if((pool = (printpool_t *)malloc(sizeof(printpool_t))) == NULL) {
        //ERROR MESSAGE
    }

    pool->aantal_printers = 0;
    pool->aantal_taken = info.aantal_taken;
    pool->volgende = pool->laatste = pool->huidig_taken = 0;
    pool->printers = (pthread_t *)malloc(sizeof(pthread_t) * info.aantal_printers);
    pool->taken = (printpool_taak *)malloc
        (sizeof(printpool_taak) * info.aantal_taken);

    if((pthread_mutex_init(&(pool->bezig), NULL) != 0) ||
       (pthread_cond_init(&(pool->beschikbaar), NULL) != 0) ||
       (pool->printers == NULL) ||
       (pool->taken == NULL)) {
        //TODO: ERR MESSAGE
    }

    printf("Printerpool size = %d:\n", info.aantal_printers);
    printerinfos = malloc(sizeof(printerinfo_t) * info.aantal_printers);
    for(i = 0; i < info.aantal_printers; i++) {
        printerinfos[i] = (printerinfo_t*) malloc(sizeof(printerinfo_t));
        printerinfos[i]->host = malloc(sizeof(info.hosts[i]));
        printerinfos[i]->port = malloc(sizeof(info.ports[i]));
        printerinfos[i]->printpool = malloc(sizeof(pool));
        
        strncpy(printerinfos[i]->host, info.hosts[i], strlen(info.hosts[i]));
        strncpy(printerinfos[i]->port, info.ports[i], strlen(info.ports[i]));
        printerinfos[i]->printpool = pool;

        if(pthread_create(&(pool->printers[i]), NULL,
                          printer, (void*)printerinfos[i]) != 0) {
            return NULL;
        }
        pool->aantal_printers++;
    }
    return pool;
}

void printpool_nieuwe_taak(printpool_t *pool, char filenaam[10])
{
    int next;

    if(pool == NULL || filenaam == NULL) {
        fprintf(stderr, "Pool or taak is non existent!");
    }

    if(pthread_mutex_lock(&(pool->bezig)) != 0) {
        fprintf(stderr, "Can't bezig Mutex!");
    }

    next = pool->laatste + 1;
    next = (next == pool->aantal_taken) ? 0 : next;

    do {
        if(pool->huidig_taken == pool->aantal_taken) {
            fprintf(stderr, "Job list full!");
            break;
        }

        strncpy(pool->taken[pool->laatste].filenaam, filenaam, strlen(filenaam));
        pool->laatste = next;
        pool->huidig_taken += 1;

        if(pthread_cond_signal(&(pool->beschikbaar)) != 0) {
            fprintf(stderr, "Signal Available!");
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->bezig) != 0) {
        fprintf(stderr, "Can't Unlock Mutex!");
    }
}

static void *printer(void *printerinfo)
{
    printerinfo_t *pi = (printerinfo_t *) printerinfo; 
    printpool_t *pool = (printpool_t *) pi->printpool;
    printpool_taak taak;

    fprintf(stdout, "\tPrinter[%s:%s] succesfully started.\n", pi->host, pi->port);
    // TODO: CONNECTION WITH PRINTER HERE


    while(1) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->bezig));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the bezig. */
        while((pool->huidig_taken == 0)) {
            pthread_cond_wait(&(pool->beschikbaar), &(pool->bezig));
        }

        /* Grab our taak */
        strcpy(taak.filenaam,pool->taken[pool->volgende].filenaam);
        pool->volgende += 1;
        pool->volgende = (pool->volgende == pool->aantal_taken) ? 0 : pool->volgende;
        pool->huidig_taken -= 1;

        /* Unlock */
        pthread_mutex_unlock(&(pool->bezig));

        /* Get to work */

        //INSTEAD OF FUNCTION TAKE FILENAME AND SEND TO PRINTER WAIT FOR RESPONSE
        fprintf(stderr, "%s", taak.filenaam);
    }

    pthread_mutex_unlock(&(pool->bezig));
    pthread_exit(NULL);
    return(NULL);
}
