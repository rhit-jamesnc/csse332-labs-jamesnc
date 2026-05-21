/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/21/26
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int occupants = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_enter = PTHREAD_COND_INITIALIZER;

void *
thread(void *arg)
{
  char *letter = (char *)arg;
  printf("%c wants to enter the critical section\n", *letter);

  pthread_mutex_lock(&lock);
  while (occupants >= 3) {
    pthread_cond_wait(&can_enter, &lock);
  }
  occupants++;
  pthread_mutex_unlock(&lock);

  printf("%c is in the critical section\n", *letter);
  sleep(1);
  
  pthread_mutex_lock(&lock);
  printf("%c has left the critical section\n", *letter);
  occupants--;
  pthread_cond_signal(&can_enter);
  pthread_mutex_unlock(&lock);

  return NULL;
}

int
main(int argc, char **argv)
{
  pthread_t threads[8];
  int i;
  char *letters = "abcdefgh";

  for(i = 0; i < 8; ++i) {
    pthread_create(&threads[i], NULL, thread, &letters[i]);

    if(i == 4)
      sleep(4);
  }

  for(i = 0; i < 8; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Everything finished...\n");

  return 0;
}
