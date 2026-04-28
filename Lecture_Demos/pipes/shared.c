#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  pid_t pid;
  int *array;

  void *p = mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(p == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  if((pid = fork()) == 0) {
    // Child
    array = (int *)p;

    sleep(1);
    array[0] = 1;
    array[1] = 2;
    array[2] = 3;

    exit(0);
  }

  // Parent
  // wait(0);
  sleep(1);

  array = (int *)p;
  for(int i = 0; i < 3; i++) {
    printf("array[%d] = %d\n", i, array[i]);
  }

  exit(0);
}
