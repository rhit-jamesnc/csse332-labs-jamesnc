/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/21/26
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
int thread_to_run = 1;

void *
thread(void *arg)
{
  int *num = (int *)arg;
  printf("%d wants to enter the critical section\n", *num);

  pthread_mutex_lock(&lock);
  while (*num != thread_to_run) {
    pthread_cond_wait(&cv, &lock);
  }

  // simulate the critical section with a simple sleep.
  sleep(1);

  thread_to_run++;
  pthread_cond_broadcast(&cv);
  pthread_mutex_unlock(&lock);

  printf("%d is finished with the critical section\n", *num);

  return NULL;
}

int
main(int argc, char **argv)
{
  pthread_t threads[4];
  int i;
  int nums[] = {2, 1, 4, 3};

  for(i = 0; i < 4; ++i) {
    pthread_create(&threads[i], NULL, thread, &nums[i]);

    if(i == 2)
      sleep(3);
  }

  for(i = 0; i < 4; ++i) {
    pthread_join(threads[i], NULL);
  }

  printf("Everything finished\n");

  return 0;
}
