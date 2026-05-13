/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date  5/13/26
 */

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include "rf_proc.h"
#include "rf_sched.h"
#include "rf_parse.h"

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

  p->state = RF_STATE_RUNNING;
  int (*entry_func)() = (int (*)())p->entry;
  p->proc_rv = entry_func();
  p->state = RF_STATE_ZOMBIE;

  printf("Process %s completed and returned %d\n", p->name, p->proc_rv);

  swapcontext(&p->ctx, p->sched);
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

  struct rf_proc *new_proc = NULL;

  // 1. Find a valid process spot in our array.
  for (int i = 0; i < NPROC; i++) {
    if (procs[i].state == RF_STATE_UNUSED) {
      new_proc = &procs[i];
      break;
    }
  }

  // If the table is full (16 procs), we can't add more.
  if (new_proc == NULL) {
    fprintf(stderr, "[ERROR] Could not map a new process!\n");
    return NULL;
  }

  // 2. Extract the process name (Safely!)
  // We start at the end and walk backward. 
  // CRITICAL: We must check *(pname - 1) to look at the character before.
  const char *pname = path + strlen(path);
  while(pname != path && *(pname - 1) != '/') {
    pname--;
  }

  // 3. Load the process's memory address space.
  // We increment the global pid for every successfully loaded process.
  if (load_proc(new_proc, path, pid++, pname) != 0) {
    new_proc->state = RF_STATE_UNUSED; 
    return NULL;
  }
  
  // 4. Link the scheduler context.
  // This ensures the wrapper knows where to go when the process finishes.
  new_proc->sched = &sched; 

  // 5. Setup the execution context.
  // makecontext modifies the context so it starts at proc_wrapper(new_proc).
  // The '1' indicates we are passing one argument (new_proc).
  makecontext(&new_proc->ctx, (void (*)(void))proc_wrapper, 1, new_proc);

  new_proc->state = RF_STATE_READY;

  return new_proc;
}

void
run_sched(void)
{
  // TODO:
  // =====
  //  Implement the process scheduler here. Make sure to keep running
  //  everything, one by one, until all processes are finished.
  //
  int active_procs = 1;

  while (active_procs > 0) {
    active_procs = 0;

    for (int i = 0; i < NPROC; i++) {
      if (procs[i].state == RF_STATE_READY) {
        active_procs++;
        curr = &procs[i];

        swapcontext(&sched, &procs[i].ctx);

        if (procs[i].state == RF_STATE_ZOMBIE) {
          unmap_proc(&procs[i]);
          procs[i].state = RF_STATE_UNUSED;
          active_procs--;
        }
      }
    }
  }
}
