#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"

void zem_init(zem_t *s, int value) {
  s->number=value;
  pthread_cond_init(&s->zemaphore_con,NULL);
  pthread_mutex_init(&s->zemaphore_mutex, NULL);
}

void zem_down(zem_t *s) {
  pthread_mutex_lock(&s->zemaphore_mutex);
  while(s->number<=0){
    pthread_cond_wait(&s->zemaphore_con,&s->zemaphore_mutex);
  }
  s->number--;
  pthread_mutex_unlock(&s->zemaphore_mutex);
}

void zem_up(zem_t *s) {
  pthread_mutex_lock(&s->zemaphore_mutex);
  s->number++;
  pthread_cond_signal(&s->zemaphore_con);
  pthread_mutex_unlock(&s->zemaphore_mutex);
}
