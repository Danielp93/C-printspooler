
#include <pthread.h>

typedef struct {
    void (*function)(int);
    int argument;
} printpool_taak;

struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  printpool_taak *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
};

typedef struct threadpool_t threadpool_t;

threadpool_t *printpool_init(int thread_count, int queue_size, int flags);

void printpool_nieuwe_taak(threadpool_t *pool, void (*routine)(int),
                   int arg, int flags);