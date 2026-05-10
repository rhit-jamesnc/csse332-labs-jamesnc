#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

/**
 * In this exercise, we will try to again simulate pthread_join using condition
 * variables. We will create a thread and then implement the thread_join
 * function below.
 */

pthread_mutex_t lk = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv  = PTHREAD_COND_INITIALIZER;

// state of the world
int child_done = 0;

/*
 * exit from a given thread
 */
void
thread_exit(void)
{
  /* Implement this function */
  pthread_mutex_lock(&lk);
  child_done = 1;
  pthread_cond_signal(&cv);
  pthread_mutex_unlock(&lk);
}

void *
child(void *ignored)
{
  /* modify this code to let the parent know the child is done */
  printf("Child\n");

  sleep(5);

  printf("Child completed\n");

  thread_exit();

  return 0;
}

/**
 * thread_join - Wait for the thread that we create in main by using a condition
 * variable.
 */
void
thread_join(void)
{
  /* Implement this function */
  pthread_mutex_lock(&lk);
  while(child_done != 1) {
    pthread_cond_wait(&cv, &lk);
  }
  pthread_mutex_unlock(&lk);
}

int
main(int argc, char **argv)
{
  pthread_t thread;

  pthread_create(&thread, 0, child, NULL);

  printf("Parent waiting for the child to finish\n");
  thread_join();

  /* done */
  exit(0);
}
