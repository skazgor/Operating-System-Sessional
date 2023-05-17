#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 26

zem_t zem[NUM_THREADS];

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  char ch='z'-thread_id;
  while(1)
    { zem_down(&zem[thread_id]);
      printf("%c\n", ch);
       sleep(1);
      zem_up(&zem[(thread_id+1)%NUM_THREADS]);
    }
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];

  
  for(int i =0; i < NUM_THREADS; i++)
    {
      zem_init(&zem[i],0);
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }
  zem_up(&zem[0]);
  for(int i =0; i < NUM_THREADS; i++)
    {
      pthread_join(mythreads[i], NULL);
    }
  
  return 0;
}
