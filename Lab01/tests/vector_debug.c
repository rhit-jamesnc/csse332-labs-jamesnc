/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

int
main(int argc, char **argv)
{
  struct vector *vec = vec_new();

  // Add any code you'd like to test out independently here. It will be
  // compiled and built into %_debug.run that you can load into gdb.
  printf("TODO: Add vector debug code here if needed...\n");

  printf("%d\n", vec_elem_at(vec, 1));

  vec_free(vec);
  return EXIT_SUCCESS;
}
