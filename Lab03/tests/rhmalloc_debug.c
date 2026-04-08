/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/8/26
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rhmalloc.h"

int
main(int argc, char **argv)
{
  void *p;
  size_t size = 1024;

  if(rhmalloc_init()) {
    fprintf(stderr, "[ERROR]: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // TODO:
  // =====
  //
  //  Add code you want to debug here and replace the corresponding calls.
  //
  p = rhmalloc(size);
  if(!p) {
    fprintf(stderr, "[rhmalloc error]: %s\n", strerror(errno));
    if(rhfree_all())
      fprintf(stderr, "[rhfree error]: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("Used rhmalloc to allocate %lu bytes at %p\n", size, p);
  rhfree(p);

  // NOTE:
  // =====
  //  Make sure to leave this here to cleanup before you leave.
  //
  if(rhfree_all()) {
    fprintf(stderr, "[rhfree error]: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
