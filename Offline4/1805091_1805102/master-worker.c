#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size, item_to_comsume = 0;
int total_items, max_buf_size, num_workers, num_masters;
int produce_iterator = 0;
int consume_iterator = 0;

pthread_mutex_t mutex;

pthread_cond_t empty, full;

int *buffer;

void print_produced(int num, int master)
{

  printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker)
{

  printf("Consumed %d by worker %d\n", num, worker);
}
void generate_request(int req_number)
{
  if (produce_iterator == max_buf_size)
  {
    produce_iterator = 0;
  }
  curr_buf_size++;
  buffer[produce_iterator++] = req_number;
}

// produce items and place in buffer
// modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while (1)
  {
    pthread_mutex_lock(&mutex);
    if (item_to_produce >= total_items)
    {
      pthread_mutex_unlock(&mutex);
      break;
    }

    while (curr_buf_size == max_buf_size)
    {
      if (item_to_produce >= total_items)
      {
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
      }
      pthread_cond_wait(&full, &mutex);
    }
    if (item_to_produce >= total_items)
    {
      pthread_mutex_unlock(&mutex);
      break;
    }
    generate_request(item_to_produce);
    print_produced(item_to_produce, thread_id);
    item_to_produce++;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
  }
  pthread_cond_broadcast(&full);
  return 0;
}

int serve_request()
{
  if (consume_iterator == max_buf_size)
  {
    consume_iterator = 0;
  }
  curr_buf_size--;
  return buffer[consume_iterator++];
}
// write function to be run by worker threads
void *serve_request_loop(void *arg)
{
  int consumer_id = *((int *)arg);
  while (1)
  {
    pthread_mutex_lock(&mutex);
    if (item_to_comsume >= total_items)
    {
      pthread_mutex_unlock(&mutex);
      break;
    }
    while (curr_buf_size == 0)
    {
      pthread_cond_wait(&empty, &mutex);
      if (item_to_comsume >= total_items)
      {
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
      }
    }
    if (item_to_comsume >= total_items)
    {
      pthread_mutex_unlock(&mutex);
      break;
    }
    int served_req = serve_request();
    print_consumed(served_req, consumer_id);
    item_to_comsume++;
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
  }
  pthread_cond_broadcast(&empty);
  return 0;
}

int main(int argc, char *argv[])
{
  int *master_thread_id;
  pthread_t *master_thread;
  int *worker_thread_id;
  pthread_t *worker_thread;
  item_to_produce = 0;
  curr_buf_size = 0;

  int i;

  if (argc < 5)
  {
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else
  {
    num_masters = atoi(argv[4]);
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&empty, NULL);
  pthread_cond_init(&full, NULL);

  buffer = (int *)malloc(sizeof(int) * max_buf_size);

  // create master producer threads
  master_thread_id = (int *)malloc(sizeof(int) * num_masters);
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  
  for (i = 0; i < num_masters; i++)
  {
    master_thread_id[i] = i;
  }
  for (i = 0; i < max_buf_size; i++)
  {
    buffer[i] = -1;
  }
  worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);

  for (i = 0; i < num_workers; i++)
  {
    worker_thread_id[i] = i;
  }

  for (i = 0; i < num_masters; i++)
    pthread_create(&master_thread[i], NULL, &generate_requests_loop, (void *)&master_thread_id[i]);

  // create worker consumer threads
  for (i = 0; i < num_workers; i++)
    pthread_create(&worker_thread[i], NULL, &serve_request_loop, (void *)&worker_thread_id[i]);

  // wait for all threads to complete
  for (i = 0; i < num_masters; i++)
  {
    pthread_join(master_thread[i], NULL);
    printf("master %d joined\n", i);
  }
  for (i = 0; i < num_workers; i++)
  {
    pthread_join(worker_thread[i], NULL);
    printf("worker %d joined\n", i);
  }

  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);
  free(worker_thread);
  free(worker_thread_id);
  return 0;
}
