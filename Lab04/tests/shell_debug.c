/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/22/26
 */

#include <stdio.h>
#include <stdlib.h>

#include "shell.h"

#ifndef MAX_SHELL_ARGS
#define MAX_SHELL_ARGS 16
#endif

int
main(int argc, char **argv)
{
  const char *cmd = "echo hello";
  int carg        = 0;
  int i           = 0;
  char *varg[MAX_SHELL_ARGS];
  char scmd[MAX_SHELL_ARGS];

  snprintf(scmd, MAX_SHELL_ARGS, "%s", cmd);
  carg = generate_exec_args(scmd, varg);
  for(i = 0; i < carg; i++) {
    printf("argv[i] is %s\n", varg[i]);
  }
  printf("Last argument is %s\n", varg[i]);

  return EXIT_SUCCESS;
}
