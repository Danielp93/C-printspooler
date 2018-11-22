#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../printpool.d/printpool.h"

static void *printer(void *printpool);

printpool_t *printpool_init(int thread_count, int queue_size, int flags)
{
    printpool_t *pool;
    int i;

    if((pool = (printpool_t *)malloc(sizeof(printpool_t))) == NULL) {
        //ERROR MESSAGE
    }

    pool->aantal_printers = 0;
    pool->aantal_taken = queue_size;
    pool->volgende = pool->laatste = pool->huidig_taken = 0;

    pool->printers = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->taken = (printpool_taak *)malloc
        (sizeof(printpool_taak) * queue_size);

    if((pthread_mutex_init(&(pool->bezig), NULL) != 0) ||
       (pthread_cond_init(&(pool->beschikbaar), NULL) != 0) ||
       (pool->printers == NULL) ||
       (pool->taken == NULL)) {
        //TODO: ERR MESSAGE
    }

    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool->printers[i]), NULL,
                          printer, (void*)pool) != 0) {
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

        strcpy(pool->taken[pool->laatste].filenaam, filenaam);
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

static void *printer(void *printpool)
{
    printpool_t *pool = (printpool_t *)printpool;
    printpool_taak taak;


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
