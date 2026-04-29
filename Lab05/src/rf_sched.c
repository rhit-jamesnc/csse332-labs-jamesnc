/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include "rf_proc.h"
#include "rf_sched.h"

// Constants
#define NPROC 16 //!< The total number of allowed processes.

// Globals for the scheduler
unsigned pid            = 1;   //!< A tracker of used process ids
struct ucontext_t sched = {0}; //!< The scheduler context
struct rf_proc *curr    = 0;   //!< The currently running process
struct rf_proc procs[NPROC];   //!< The list of all processes in the system.

void
sched_init(void)
{
  pid = 1;
  getcontext(&sched);
  curr = 0;
  for(int i = 0; i < NPROC; i++) {
    procs[i].state = RF_STATE_UNUSED;
  }
}

struct ucontext_t *
get_sched(void)
{
  return &sched;
}

struct rf_proc *
get_current_proc(void)
{
  return curr;
}

void
set_current_proc(struct rf_proc *p)
{
  curr = p;
}

void
proc_wrapper(struct rf_proc *p)
{
  // TODO:
  // =====
  //  We need a way to force processes to call a finish function, so we will
  //  wrap each proc with this wrapper that call the process's main function,
  //  collect the output values, and then move back to the scheduler.
  //
  // 1. Call the process's entry function.
  //
  // 2. Save the process's return value.
  //
  // 3. Adjust the state of the process accordingly.
  //
  // 4. Move back to the scheduler.

  printf("Process %s completed and returned %d\n", p->name, p->proc_rv);
}

struct rf_proc *
add_process(const char *path)
{
  // TODO:
  // =====
  //
  //  This process adds a process to our list of possible processes and
  //  prepares it to be run. After calling this function, we should be able to
  //  schedule this process with any issues.
  //
  // 1. Find a valid process spot in our array.
  //
  // 2. Load the process's memory address space, but do not start it yet!
  //
  // 3. Make the process's context so that we would use our wrapper to force it
  //    to clean after itself.

  // Hint: This is here to help out cleanup the name of a process.
  // create the name for the process.
  //  search backwards from the end until we hit start or the first /
  const char *pname = path + strlen(path);
  while(pname != path && *pname != '/')
    pname--;

  return NULL;
}

void
run_sched(void)
{
  // TODO:
  // =====
  //  Implement the process scheduler here. Make sure to keep running
  //  everything, one by one, until all processes are finished.
  //
}
