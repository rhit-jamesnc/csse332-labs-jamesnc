/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/
 #include "hello.h"
 #include <stdio.h>
 #include <unistd.h>

/**
 * Implementation of print_hello_world
 */
int
print_hello_world()
{
  //void *p   = (void *)0xdeadbeef;
  //*(int *)p = 3;

  sleep(1); //autograde will time out! be careful!
  printf("Hello world!\n");

  return 3;
}
