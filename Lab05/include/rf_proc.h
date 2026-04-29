/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef RF_PROC_H
#define RF_PROC_H

#include <sys/ucontext.h>
#include <ucontext.h>

enum RF_PROC_ERR {
  RF_PROC_UNMAP_FAILED, //!< Failed while unmapping the process.
  RF_PROC_PARSE_FAILED, //!< Parsing failure, check the parse state error.
  RF_PROC_MEM_FAILED,   //!< Memory allocation failure.

  RF_PROC_ERR_CLEAR, //!< always keep this at the end.
};

/**
 * The possible states that a process can have.
 */
enum RF_PROC_STATE {
  RF_STATE_BORN = 0, //!< This state has just been born, not ready yet.
  RF_STATE_READY,    //!< A process that is ready to be executed.
  RF_STATE_RUNNING,  //!< A process that is actively running.
  RF_STATE_SLEEPING, //!< A process in sleep mode.
  RF_STATE_ZOMBIE,   //!< A process in zombie mode, dead but not cleaned up.
  RF_STATE_UNUSED,   //!< An unused struct proc.

  RF_STATE_LASTONE, //!< Leave this as the last one.
};

/**
 * A snapshot of the a process's memory map.
 */
struct rf_pmap {
  void *code;    //!< The start of the process's code region.
  void *data;    //!< The start of the process's data region.
  void *stack;   //!< The start of the process's stack region.
  unsigned clen; //!< The length in bytes of the code region.
  unsigned dlen; //!< The length in bytes of the data region.
  unsigned slen; //!< The length in bytes of the stack region.
};

/**
 * A process in Runner Format.
 */
struct rf_proc {
  unsigned pid;   //!< A unique process id for this process.
  ucontext_t ctx; //!< The context for the process to use for context switching.
  ucontext_t *sched;  //!< The context of the scheduler that brought this.
  void *entry;        //!< The address of the process's entry point.
  int state;          //!< THe current state of the process.
  struct rf_pmap map; //!< The process's current memory map.
  int proc_err;       //!< The error state of the process, if any.
  int proc_rv;        //!< The return value of the process's entry function.
  char name[32];      //!< The process's name.
};

/**
 * Initialize a proc structure to hold some default values.
 *
 * @param proc  The process structure to intialize.
 * @param pid   The process's pid to be assigned.
 * @param name  The name to give to this process.
 *
 * @return 0 on success, -1 on failure.
 *   Can only fail do to context issues, in that case, errno is set.
 */
int init_proc(struct rf_proc *proc, unsigned pid, const char *name);

/**
 * Unmap all of a process's address space.
 *
 * @param proc  The process structure to wipe out.
 *
 * @return 0 on success, -1 on failure and sets proc->proc_err.
 */
int unmap_proc(struct rf_proc *proc);

/**
 * Load a process from a CSSE332 RF File.
 *
 * @param proc  The process structure to load into.
 * @param path  The RF file to load from.
 *
 * @return 0 on success, error number on failure.
 */
int load_proc(struct rf_proc *proc, const char *path, unsigned pid,
              const char *pname);

/**
 * Set the stack values in the context for the process and mark it ready.
 *
 * @param proc  The process structure to make.
 *
 * @return nothing.
 */
void make_proc(struct rf_proc *proc);

#endif // rf_proc.h
