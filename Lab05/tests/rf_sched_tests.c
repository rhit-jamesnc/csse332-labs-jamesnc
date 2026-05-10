/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 */
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include "cgassert.h"
#include "cgrade.h"
#include "cgradescropify.h"
#include "rf_proc.h"
#include "rf_sched.h"

typedef int (*start_fn)(void);

static int
test_make_proc(void)
{
  struct rf_proc p;

  make_proc(&p);
  CG_ASSERT_INT_EQ_MSG(
      RF_STATE_READY, p.state,
      "The state of a process must be marked ready after creation.");

  return CG_TEST_PASSED;
}

static int
test_load_proc(void)
{
  struct rf_proc p;
  int err;
  start_fn start;

  err = load_proc(&p, "1.rf", 1, "test_proc");
  CG_ASSERT_INT_EQ_MSG(0, err, "Load proc for 1.rf should succeed.");

  // grab the entry function and call it
  start = (start_fn)p.entry;
  err   = start();
  CG_ASSERT_INT_EQ_MSG(5, err,
                       "Loading 1.rf into a process should run and return 5.");

  unmap_proc(&p);
  return CG_TEST_PASSED;
}

static int
test_load_proc_glbl(void)
{
  struct rf_proc p;
  int err;
  start_fn start;

  err = load_proc(&p, "3.rf", 1, "test_proc");
  CG_ASSERT_INT_EQ_MSG(0, err, "Load proc for 3.rf should succeed.");

  // grab the entry function and call it
  start = (start_fn)p.entry;
  err   = start();
  CG_ASSERT_INT_EQ_MSG(105, err,
                       "Loading 3.rf into a process should run and return 5.");

  unmap_proc(&p);
  return CG_TEST_PASSED;
}

static int
test_proc_wrapper(void)
{
  struct rf_proc p;

  load_proc(&p, "1.rf", 1, "test_proc");
  memcpy(&(p.ctx), get_sched(), sizeof p.ctx);

  cancel_swap = 1;
  proc_wrapper(&p);
  CG_ASSERT_INT_EQ_MSG(5, p.proc_rv,
                       "The proc_wrapper should run the start function, but it "
                       "did not or returned a wrong value.");
  cancel_swap = 0;

  unmap_proc(&p);
  return CG_TEST_PASSED;
}

static int
test_proc_add(void)
{
  struct rf_proc *p, *q;
  start_fn start;
  int rv;

  sched_init();
  p = add_process("1.rf");
  CG_ASSERT_PTR_NNUL_MSG(p, "add_process(1.rf) must return a valid process.");

  // check the process's state
  CG_ASSERT_INT_EQ_MSG(RF_STATE_READY, p->state,
                       "Check that the added process is in the ready state.");

  // call the starting function
  start = (start_fn)p->entry;
  rv    = start();
  CG_ASSERT_INT_EQ_MSG(
      5, rv, "Adding process from 1.rf should be runnable and return 5.");

  q = add_process("3.rf");
  CG_ASSERT_PTR_NNUL_MSG(q, "add_process(3.rf) must return a valid process.");

  // check the process's state
  CG_ASSERT_INT_EQ_MSG(RF_STATE_READY, q->state,
                       "Check that the added process is in the ready state.");

  // call the starting function
  start = (start_fn)q->entry;
  rv    = start();
  CG_ASSERT_INT_EQ_MSG(
      105, rv, "Adding process from 1.rf should be runnable and return 5.");

  CG_ASSERT_MSG(p != q,
                "Adding two processes should go into two different spots");

  unmap_proc(p);
  unmap_proc(q);
  return CG_TEST_PASSED;
}

int
test_add_process_invalid(void)
{
  sched_init();
  struct rf_proc *p = add_process("nonexistent.rf");
  CG_ASSERT_PTR_EQ_MSG(
      NULL, p, "add_process must return NULL when the file does not exist");
  return CG_TEST_PASSED;
}

int
test_sched_one_proc(void)
{
  struct rf_proc *p;

  sched_init();
  p = add_process("1.rf");
  run_sched();

  // check the content of p after finishing up.
  CG_ASSERT_INT_EQ_MSG(
      RF_STATE_UNUSED, p->state,
      "Completed process should be left unused after it is done.");
  CG_ASSERT_INT_EQ_MSG(5, p->proc_rv,
                       "Completed process from 1.rf should return 5.");

  return CG_TEST_PASSED;
}

int
test_sched_another_proc(void)
{
  struct rf_proc *p;

  sched_init();
  p = add_process("3.rf");

  // modify the globals array
  *(int *)p->map.data = 200;

  run_sched();

  // check the content of p after finishing up.
  CG_ASSERT_INT_EQ_MSG(
      RF_STATE_UNUSED, p->state,
      "Completed process should be left unused after it is done.");
  CG_ASSERT_INT_EQ_MSG(205, p->proc_rv,
                       "Completed process from 1.rf should return 5.");

  return CG_TEST_PASSED;
}

static int
test_multiple_procs_run_and_complete(void)
{
  int return_vals[] = {5, 5, 105, 105, 294, 983076, 983076};
  const char *rfs[] = {"1.rf", "2.rf", "3.rf", "4.rf", "5.rf", "6.rf", "7.rf"};
  struct rf_proc *_procs[10];

  sched_init();
  for(int i = 0; i < 10; i++) {
    _procs[i] = add_process(rfs[i % 7]);
    CG_ASSERT_PTR_NNUL_MSG(_procs[i], "Adding process %d for %s return NULL.",
                           i + 1, rfs[i % 7]);
  }

  run_sched();

  for(int i = 0; i < 10; i++) {
    CG_ASSERT_INT_EQ_MSG(return_vals[i % 7], _procs[i]->proc_rv,
                         "Checking process %d (%s) returned wrong return value",
                         i + 1, rfs[i % 7]);
    CG_ASSERT_INT_EQ_MSG(
        RF_STATE_UNUSED, _procs[i]->state,
        "Checking process %d(%s): should be unused after completed", i + 1,
        rfs[i % 7]);
  }

  return CG_TEST_PASSED;
}

static struct cg_test_suite *
rf_load_test_suite(void)
{
  struct cg_test_suite *ts = cg_test_suite_new("RF Scheduler Tests", 0);
  if(!ts)
    exit(EXIT_FAILURE);

  // test_make_proc, 5 points
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_make_proc", test_make_proc, 5,
                              "Check that make_proc behaves correctly");

  // test_load_proc, 10 points
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_load_proc", test_load_proc, 10,
                              "Check that load_proc can run a proces");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_load_proc_glbl", test_load_proc_glbl,
                              10, "Check that load_proc gets globals right.");

  // test_proc_wrapper
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_proc_wrapper", test_proc_wrapper, 10,
      "Check the proc_wrapper can run a process in the same Scheduler");

  // test_add_proc
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_proc_add", test_proc_add, 15,
      "Check that we can add a process correctly using add_process");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_add_process_invalid", test_add_process_invalid, 10,
      "Check that adding an invalid process returns a NULL pointer");

  // test_sched
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_sched_one_proc", test_sched_one_proc,
                              10, "Check that we can run a single process");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_sched_another_proc",
                              test_sched_another_proc, 10,
                              "Check that we can another process");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_multiple_procs_run_and_complete",
      test_multiple_procs_run_and_complete, 20,
      "Check that we can run multiple processes correctly");

  return ts;
}

int
main(int argc, char **argv)
{
#ifdef BUILD_GRADESCOPE
  struct cg_test_suite *ts = rf_load_test_suite();
  int rc                   = cg_test_suite_runall(ts);

  cg_test_suite_summarize(ts);
  cg_test_suite_gradescopify_tests(ts, "rf_sched_tests.run.json");

  cg_test_suite_remove(ts);
  return (rc > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
#else
  int rc                  = 0;
  struct cg_project *proj = cg_project_new("RF Scheduler");
  cg_project_add_suite(proj, rf_load_test_suite());

  cg_project_runall(proj);
  cg_project_summarize(proj);

  rc = proj->num_failures > 0 ? 1 : 0;
  cg_project_del(proj);
  return rc;
#endif
}
