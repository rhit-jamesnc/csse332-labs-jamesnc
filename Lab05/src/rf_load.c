/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "rf_load.h"
#include "rf_parse.h"

int
calc_npages(struct rf_parse_state *ps)
{
  // TODO:
  // =====
  //  Implement this function to compute how many pages we'd need to store the
  //  code and globals regions of the CSSE332RF binary in question.
  return 0;
}

void *
rf_load_code(struct rf_parse_state *ps, void *code, int *len)
{
  // TODO:
  // =====
  //  Implement code that would load the code segment from the file into the
  //  area of memory starting at location code.
  return 0;
}

int
rf_exec_code_only(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int err                  = 0;

  // Load the parser state
  if(rf_parse_state_init(&ps, path)) {
    return ps.error;
  }

  // Read and validate the header
  if(rf_read_hdr(&ps, &hdr) || rf_validate_hdr(&hdr, &ps)) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 1. Load the code segment from the binary.

  // 2. Find the entry point for the newly created virtual process.

  // 3. Execute the process by simply calling the entry function and capturing
  //    the return value. Save the return value into `exst->rv`.

  // DO NOT unmap the execution state, we do that in rf_unmap_state
  rf_parse_state_destroy(&ps);
  return 0;
}

void *
rf_load_data(struct rf_parse_state *ps, void *code, int *len)
{
  // TODO:
  // =====
  //  Implement code that would load the globals segment (if any) from the file
  //  into the page that starts right after the code region. Make sure to stay
  //  page aligned.
  return 0;
}

int
rf_exec(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int err                  = 0;

  // Load the parser state
  if(rf_parse_state_init(&ps, path)) {
    return ps.error;
  }

  // Read and validate the header
  if(rf_read_hdr(&ps, &hdr) || rf_validate_hdr(&hdr, &ps)) {
    err = ps.error;
    rf_parse_state_destroy(&ps);
    return err;
  }

  // 1. Adjust memory so that we respect the locations of the code and globals
  //    required by the CSSE332 RF format.

  // 2. Load the code segment from the binary.

  // 3. Load the globals segment (if any) from the binary into memory at a
  //    specific location.

  // 4. Find the entry point for the newly created virtual process.

  // 5. Execute the process by simply calling the entry function and capturing
  //    the return value. Save the return value into `exst->rv`.

  // DO NOT unmap the execution state, we do that in rf_unmap_state
  rf_parse_state_destroy(&ps);
  return 0;
}

int
rf_unmap_state(struct rf_exec_state *exst)
{
  int err = 0;
  if(exst->code) {
    err = munmap(exst->code, exst->clen);
    if(err) {
      perror("munmap");
      return -1;
    }
  }
  if(exst->data) {
    err = munmap(exst->data, exst->dlen);
    if(err) {
      perror("munmap");
      return -1;
    }
  }

  return err;
}
