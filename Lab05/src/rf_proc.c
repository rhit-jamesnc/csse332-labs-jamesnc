/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

#include "rf.h"
#include "rf_load.h"
#include "rf_parse.h"
#include "rf_proc.h"

int
init_proc(struct rf_proc *proc, unsigned int pid, const char *name)
{
  proc->pid   = pid;
  proc->sched = 0;
  memset(&proc->map, 0, sizeof(struct rf_pmap));
  strncpy(proc->name, name, 31);
  proc->state    = RF_STATE_BORN;
  proc->proc_err = RF_PROC_ERR_CLEAR;

  // Initialize the context for this process.
  if(getcontext(&proc->ctx)) {
    return -1;
  }
  return 0;
}

int
unmap_proc(struct rf_proc *proc)
{
  int rc         = 0;
  // unmap memory regions one by one.
  //  if we fail, it's ok, still try the others but report error back.
  proc->proc_err = RF_PROC_ERR_CLEAR;
  if(proc->map.code) {
    if(munmap(proc->map.code, proc->map.clen)) {
      rc             = -1;
      proc->proc_err = RF_PROC_UNMAP_FAILED;
    }
  }
  if(proc->map.data) {
    if(munmap(proc->map.data, proc->map.dlen)) {
      rc             = -1;
      proc->proc_err = RF_PROC_UNMAP_FAILED;
    }
  }
  if(proc->map.stack) {
    if(munmap(proc->map.stack, proc->map.slen)) {
      rc             = -1;
      proc->proc_err = RF_PROC_UNMAP_FAILED;
    }
  }
  return rc;
}

void
make_proc(struct rf_proc *proc)
{
  // init_proc should have been called before that and all the memory mapping
  // have been created.
  proc->ctx.uc_stack.ss_sp   = proc->map.stack;
  proc->ctx.uc_stack.ss_size = proc->map.slen;

  // TODO:
  // =====
  //  Set the state of the process to be ready!
}

int
load_proc(struct rf_proc *proc, const char *path, unsigned pid,
          const char *pname)
{
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};

  // Initialize the process
  if(init_proc(proc, pid, pname)) {
    proc->proc_err = RF_PROC_PARSE_FAILED;
    return -1;
  }

  // Load the parser
  if(rf_parse_state_init(&ps, path)) {
    proc->proc_err = RF_PROC_PARSE_FAILED;
    return ps.error;
  }

  // Read the header
  if(rf_read_hdr(&ps, &hdr)) {
    proc->proc_err = RF_PROC_PARSE_FAILED;
    return ps.error;
  }

  // Validate the header
  if(rf_validate_hdr(&hdr, &ps)) {
    proc->proc_err = RF_PROC_PARSE_FAILED;
    return ps.error;
  }

  // 1. Reserve an area of memory so we set up the address space

  // 2. Load the code section into memory at the right spot

  // 3. Load the data region

  // 4. Calculate the entry point

  // 5. Create a stack page

  // 6. assign the maps

  // 7. create the process
  make_proc(proc);

  // clean up
  rf_parse_state_destroy(&ps);
  return 0;
}
