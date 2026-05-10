/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef RF_SCHED_H
#define RF_SCHED_H

// forward declaration
struct rf_proc;
struct ucontext_t;

/**
 * Get a reference to the current scheduler context.
 *
 * @return A pointer to the ucontext_t structure for the scheduler itself.
 */
struct ucontext_t *get_sched(void);

/**
 * Get a reference to the currently running process, if any.
 *
 * @return A pointer to the currently running process, 0 if the scheduler is
 * running.
 */
struct rf_proc *get_current_proc(void);

/**
 * Update the currently running process.
 *
 * @param p  The process structure to set as the currently running.
 */
void set_current_proc(struct rf_proc *p);

/**
 * Initialize all the scheduler information
 */
void sched_init(void);

/**
 * Add a new process to the scheduler.
 *
 * @param path  The path of the binary to create the process from.
 *
 * @return a valid struct proc on success, 0 on failure.
 */
struct rf_proc *add_process(const char *path);

/**
 * Run the scheduler after adding processes.
 */
void run_sched(void);

/**
 * A wrapper function to force processes to clean up after finishing the user's
 * code.
 *
 * @param p   The process to wrap around.
 *
 * @return nothing.
 */
void proc_wrapper(struct rf_proc *p);

volatile int cancel_swap = 0;

#endif // rf_sched.h
