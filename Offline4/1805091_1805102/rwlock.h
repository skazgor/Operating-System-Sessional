#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>
#include <pthread.h>

using namespace std;

struct read_write_lock
{
    int readers;
    pthread_mutex_t reader_lock;
    pthread_mutex_t is_writer_waiting_lock;
    pthread_cond_t reader_wait;
    sem_t writer_lock;
    int is_writer_waiting;
};

void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
