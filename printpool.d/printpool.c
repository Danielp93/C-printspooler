#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "../printpool.d/printpool.h"

static void *printer(void *printpool);

printpool_t *printpool_init(int thread_count, int queue_size, int flags)
{
    printpool_t *pool;
    int i;

    if((pool = (printpool_t *)malloc(sizeof(printpool_t))) == NULL) {
        //ERROR MESSAGE
    }

    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (printpool_taak *)malloc
        (sizeof(printpool_taak) * queue_size);

    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        //TODO: ERR MESSAGE
    }

    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool->threads[i]), NULL,
                          printer, (void*)pool) != 0) {
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;
}

void printpool_nieuwe_taak(printpool_t *pool, void (*function)(int),
                   int argument, int flags)
{
    int next;

    if(pool == NULL || function == NULL) {
        fprintf(stderr, "Pool or taak is non existent!");
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        fprintf(stderr, "Can't lock Mutex!");
    }

    next = pool->tail + 1;
    next = (next == pool->queue_size) ? 0 : next;

    do {
        if(pool->count == pool->queue_size) {
            fprintf(stderr, "Job list full!");
            break;
        }

        if(pool->shutdown) {
            fprintf(stderr, "Can't Unlock Mutex!");
            break;
        }

        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        if(pthread_cond_signal(&(pool->notify)) != 0) {
            fprintf(stderr, "Signal Available!");
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->lock) != 0) {
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
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        while((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        /* Grab our taak */
        taak.function = pool->queue[pool->head].function;
        taak.argument = pool->queue[pool->head].argument;
        pool->head += 1;
        pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
        pool->count -= 1;

        /* Unlock */
        pthread_mutex_unlock(&(pool->lock));

        /* Get to work */

        //INSTEAD OF FUNCTION TAKE FILENAME AND SEND TO PRINTER WAIT FOR RESPONSE
        (*(taak.function))(taak.argument);
    }

    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}
