#include "rwlock.h"
#include <semaphore.h>

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
   rw->readers = 0;
   pthread_mutex_init(&rw->reader_lock, NULL);
   sem_init(&rw->writer_lock, 0, 1);
}

void ReaderLock(struct read_write_lock * rw)
{
 pthread_mutex_lock(&rw->reader_lock);
 if(rw->readers == 0)
    sem_wait(&rw->writer_lock);
  rw->readers++;
  pthread_mutex_unlock(&rw->reader_lock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  pthread_mutex_lock(&rw->reader_lock);
  rw->readers--;
  if(rw->readers == 0)
    sem_post(&rw->writer_lock);
  pthread_mutex_unlock(&rw->reader_lock);
}

void WriterLock(struct read_write_lock * rw)
{
   sem_wait(&rw->writer_lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
    sem_post(&rw->writer_lock);
}
