/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/11/26
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <pthread.h>

/**
 * In this exercise, we would like to implement a barrier to make all of our
 * threads wait at a certain location in the code before they can move formward.
 * No thread can get passed the barrier until all threads reach that barrier,
 * then they can move out together in any order.
 */ 

#define NUM_THREADS 5

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
int threads_waiting = 0;

void
barrier_wait(void)
{
  // TODO: Add your code here.
  pthread_mutex_lock(&lock);
  threads_waiting++;

  if (threads_waiting == NUM_THREADS) {
    pthread_cond_broadcast(&cv);
  } else {
    while (threads_waiting < NUM_THREADS) {
      pthread_cond_wait(&cv, &lock);
    }
  }
  

  pthread_mutex_unlock(&lock);
}

void *
thread_fn(void *arg)
{
  int tid = *(int *)arg;

  printf("Thread %d started ...\n", tid);

  sleep(rand() % 10);

  printf("Thread %d waiting at barrier ...\n", tid);
  barrier_wait();
  printf("Thread %d left the barrier ...\n", tid);

  return NULL;
}

int
main()
{
  pthread_t threads[NUM_THREADS];
  int tids[NUM_THREADS];
  int i;

  srand(time(NULL));

  for(i = 0; i < NUM_THREADS; i++) {
    tids[i] = i;
    pthread_create(&threads[i], NULL, thread_fn, &tids[i]);

    if(i == NUM_THREADS / 2)
      sleep(2);
  }

  for(i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("All done, goodbye...\n");
  exit(0);
}
