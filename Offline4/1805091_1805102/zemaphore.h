#include <pthread.h>

typedef struct zemaphore {
    int number;
    pthread_cond_t zemaphore_con;
    pthread_mutex_t zemaphore_mutex;
} zem_t;

void zem_init(zem_t *, int);
void zem_up(zem_t *);
void zem_down(zem_t *);
