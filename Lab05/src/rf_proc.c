/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date  5/13/26
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
  proc->ctx.uc_stack.ss_sp   = proc->map.stack;
  proc->ctx.uc_stack.ss_size = proc->map.slen;

  proc->state = RF_STATE_READY;
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
  size_t pagesize = getpagesize();
  int total_pages = calc_npages(&ps);

  size_t total_mem_size = total_pages * pagesize;
  void *base_ptr = mmap(NULL, total_mem_size, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // 2. Load the code section into memory at the right spot
  int clen, dlen;
  proc->map.code = rf_load_code(&ps, base_ptr, &clen);
  proc->map.clen = (unsigned)clen;

  // 3. Load the data region
  proc->map.data = rf_load_data(&ps, base_ptr, &dlen); 
  proc->map.dlen = (unsigned)dlen;

  // 4. Calculate the entry point
  proc->entry = (char *)proc->map.code + hdr.entry_offset;

  // 5. Create a stack page
  proc->map.slen = pagesize * 4; 
  proc->map.stack = mmap(NULL, proc->map.slen, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // 6. assign the maps

  // 7. create the process
  make_proc(proc);

  // clean up
  if (ps.fd >= 0) {
    close(ps.fd);
    ps.fd = -1;
  }
  return 0;
}
