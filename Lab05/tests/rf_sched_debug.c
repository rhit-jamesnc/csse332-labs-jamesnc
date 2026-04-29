/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */

#include <stdio.h>
#include <stdlib.h>

#include "rf_proc.h"
#include "rf_sched.h"

int
main(int argc, char **argv)
{
  int rfs[] = {1, 2, 3, 4, 5, 6, 7};
  char path[32];
  struct rf_proc *p;

  printf("++ Initializing scheduler: ");
  sched_init();
  printf("ok.\n");

  for(int i = 0; i < 7; i++) {
    snprintf(path, 32, "%d.rf", rfs[i]);
    printf("++ Adding process from %s:", path);
    if((p = add_process(path)) == 0) {
      printf(" failed!\n");
      fprintf(stderr, " Creation of a new process failed.\n");
      return EXIT_FAILURE;
    }
    printf(" ok (%d).\n", p->pid);
  }

  printf("++ Starting the scheduler and giving up execution.\n");
  run_sched();

  return EXIT_SUCCESS;
}
