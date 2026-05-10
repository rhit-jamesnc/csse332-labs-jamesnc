#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
main()
{
  int pid = fork();

  if(pid < 0) {
    perror("Fork failed.\n");
    return 1; // nonzero means a failure result in unix
  }

  if(pid == 0) {
    execlp("./buffalosay.run", "buffalosay.run", "Hi", NULL);
    perror("PANIC on exec");
    exit(EXIT_FAILURE);
    // alternatively:
    // char *args[] = {"buffalosay.run", "Hi", NULL};
    // execvp("./buffalosay.run", args);

    // When will this line of code ever execute?
  }

  printf("Parent: Waiting for child %d\n", pid);
  wait(0);
  printf("Parent: Child is done, goodbye...\n");
  return 0;
}
