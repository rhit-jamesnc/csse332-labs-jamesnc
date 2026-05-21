/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/21/26
 */
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Here is some code that factors in a super dumb way.  We won't be
// attempting to improve the algorithm in this case (though that would be
// the correct thing to do).
//
// Modify the code so that it starts the specified number of threads and
// splits the computation among them.  You can be sure the max allowed
// number of threads is 50.  Be sure your threads actually run in parallel.
//
// Your threads should each just print the factors they find, they don't
// need to communicate the factors to the original thread.

unsigned long long int target;
int numThreads;

// The thread worker function
void* factor_worker(void* arg) {
  long rank = (long)arg;
  unsigned long long int i;

  printf("running thread\n");

  for(i = 2 + rank; i <= target / 2; i = i + numThreads) {
    printf("thread %ld testing %llu\n", rank + 1, i);
    if(target % i == 0) {
      printf("%llu is a factor\n", i);
    }
  }
  return NULL;
}

int
main(void)
{
  printf("Give a number to factor.\n");
  scanf("%llu", &target);

  printf("How man threads should I create?\n");
  scanf("%d", &numThreads);
  if(numThreads > 50 || numThreads < 1) {
    printf("Bad number of threads!\n");
    return 0;
  }

  pthread_t threads[50];

  for(long t = 0; t < numThreads; t++) {
    pthread_create(&threads[t], NULL, factor_worker, (void*)t);
  }

  for(int t = 0; t < numThreads; t++) {
    pthread_join(threads[t], NULL);
  }

  return 0;
}
