/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 */

#define _GNU_SOURCE
#include "rf_sched.h"

#include <dlfcn.h>
#include <ucontext.h>

int
swapcontext(ucontext_t *restrict oucp, const ucontext_t *ucp)
{
  static int (*__real_wrapcontext)(ucontext_t *, const ucontext_t *) = 0;

  if(!__real_wrapcontext) {
    __real_wrapcontext = dlsym(RTLD_NEXT, "swapcontext");
  }

  if(cancel_swap) {
    return 0;
  }
  return __real_wrapcontext(oucp, ucp);
}
