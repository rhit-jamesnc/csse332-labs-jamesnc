/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "rf_load.h"
#include "rf_parse.h"

void
debug_calc_npages(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  int npages = 0;

  rf_parse_state_init(&st, "6.rf");
  rf_read_hdr(&st, &hdr);
  rf_validate_hdr(&hdr, &st);
  npages = calc_npages(&st);

  printf("Calculated %d pages!\n", npages);
}

void
debug_load_code(void)
{
  // The examples in 1.rf, 2.rf, and 7.rf all contain code-only RF binaries.
  struct rf_parse_state st;
  struct rf_hdr hdr;
  int len    = 0;
  void *code = 0;

  rf_parse_state_init(&st, "7.rf");
  rf_read_hdr(&st, &hdr);
  rf_validate_hdr(&hdr, &st);

  code = rf_load_code(&st, 0, &len);
  printf("The code segment has been loaded into %p\n", code);

  len = munmap(code, len);
  if(len < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
}

void
debug_exec(int argc, char **argv)
{
  int err;
  const char *path;
  struct rf_exec_state exst;
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <.rf file path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  path = argv[1];
  err  = rf_exec(path, &exst);
  if(err) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(err));
    exit(EXIT_FAILURE);
  }
  printf("We ran the process from %s and it had %d as return value.\n", path,
         exst.rv);

  if(rf_unmap_state(&exst)) {
    fprintf(stderr, "[ERROR] Unmap execution state failed!\n");
    exit(EXIT_FAILURE);
  }
}

int
main(int argc, char **argv)
{
  // TODO
  // ====
  //  Comment or uncomment parts of this file to debug different functions.
  //

  // debug_calc_npages();
  debug_exec(argc, argv);

  return 0;
}
