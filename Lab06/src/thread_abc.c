/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/21/26
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

//         INSTRUCTION
// Use condition variables to make  A, B, C print out in order.
// HINT: You need more than one condition variables

int turn = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_B = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_C = PTHREAD_COND_INITIALIZER;

void *
thread_func_A(void *arg)
{
  sleep(3);

  pthread_mutex_lock(&lock);

  printf("A\n");

  turn = 1;
  pthread_cond_signal(&cond_B);
  pthread_mutex_unlock(&lock);

  return NULL;
}

void *
thread_func_B(void *arg)
{
  sleep(2);

  pthread_mutex_lock(&lock);

  while (turn != 1) {
    pthread_cond_wait(&cond_B, &lock);
  }

  printf("B\n");

  turn = 2;
  pthread_cond_signal(&cond_C);
  pthread_mutex_unlock(&lock);

  return NULL;
}
void *
thread_func_C(void *arg)
{
  pthread_mutex_lock(&lock);

  while (turn != 2) {
    pthread_cond_wait(&cond_C, &lock);
  }

  printf("C\n");

  pthread_mutex_unlock(&lock);

  return NULL;
}

int
main(int argc, char *argv[])
{
  pthread_t tA, tB, tC;

  pthread_create(&tC, NULL, thread_func_C, NULL);
  pthread_create(&tB, NULL, thread_func_B, NULL);
  pthread_create(&tA, NULL, thread_func_A, NULL);

  // join waits for the threads to finish
  pthread_join(tA, NULL);
  pthread_join(tB, NULL);
  pthread_join(tC, NULL);
  return 0;
}
