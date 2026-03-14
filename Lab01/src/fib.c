/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/
#include "fib.h"

unsigned long
fib(unsigned long n)
{
  if(n <= 1)
    return n; //base case

  return fib(n - 1) + fib(n - 2);
}
