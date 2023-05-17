#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock *rw)
{
  rw->readers = 0;
  rw->is_writer_waiting = 0;
  pthread_mutex_init(&rw->reader_lock, NULL);
  pthread_mutex_init(&rw->is_writer_waiting_lock, NULL);
  pthread_cond_init(&rw->reader_wait, NULL);
  sem_init(&rw->writer_lock, 0, 1);
}
void ReaderLock(struct read_write_lock *rw)
{
  pthread_mutex_lock(&rw->is_writer_waiting_lock);
  while (rw->is_writer_waiting)
  {
    pthread_cond_wait(&rw->reader_wait, &rw->is_writer_waiting_lock);
  }
  pthread_mutex_unlock(&rw->is_writer_waiting_lock);
  pthread_mutex_lock(&rw->reader_lock);
  rw->readers++;
  if (rw->readers == 1)
  {
    sem_wait(&rw->writer_lock);
  }
  pthread_mutex_unlock(&rw->reader_lock);
}

void ReaderUnlock(struct read_write_lock *rw)
{
  pthread_mutex_lock(&rw->reader_lock);
  rw->readers--;
  if (rw->readers == 0)
  {
    sem_post(&rw->writer_lock);
  }
  pthread_mutex_unlock(&rw->reader_lock);
}

void WriterLock(struct read_write_lock *rw)
{
  pthread_mutex_lock(&rw->is_writer_waiting_lock);
  rw->is_writer_waiting = 1;
  pthread_mutex_unlock(&rw->is_writer_waiting_lock);
  sem_wait(&rw->writer_lock);
  pthread_mutex_lock(&rw->is_writer_waiting_lock);
  rw->is_writer_waiting = 0;
  pthread_cond_broadcast(&rw->reader_wait);
  pthread_mutex_unlock(&rw->is_writer_waiting_lock);
}

void WriterUnlock(struct read_write_lock *rw)
{
  sem_post(&rw->writer_lock);
}
