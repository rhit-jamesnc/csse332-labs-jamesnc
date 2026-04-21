/**
 * Copyright (c) 2026 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/20/26
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vm.h"

int
main(int argc, char **argv)
{
  void *addr = 0;
  int *array = 0;

  if(init_vm()) {
    perror("init vm failed:");
    exit(EXIT_FAILURE);
  }

  if((addr = give_me_pages(NUM_VIRT_PAGES)) == 0) {
    perror("failed to get an address:");
    teardown_vm();
    exit(EXIT_FAILURE);
  }

  printf("Obtained address %p from give_me_pages!\n", addr);

  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++) {
    // Let's put an array of 10 integers in there.
    array = (int *)(addr + pg * getpagesize());
    printf("Accessing page %02d at address %p\n", pg, array);
    for(int i = 0; i < 10; i++) {
      array[i] = i;
    }
  }

  // done, let's break things down
  teardown_vm();

  return EXIT_SUCCESS;
}
