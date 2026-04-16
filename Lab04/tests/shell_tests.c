/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <stdlib.h>
#include <unistd.h>

#include "cgassert.h"
#include "cgproject.h"
#include "cgrade.h"

#include "shell.h"

#define MAX_SHELL_ARGS 16

static int
test_generate_exec_args_ls(void)
{
  int argc;
  char *argv[MAX_SHELL_ARGS];

  argc = generate_exec_args("ls", argv);
  CG_ASSERT_INT_EQ_MSG(1, argc, "Your argv array contains %d elements!", argc);

  CG_ASSERT_STR_EQ_MSG("ls", argv[0],
                       "Your first argument in argv is %s, it is incorrect",
                       argv[0]);
  CG_ASSERT_PTR_EQ_MSG(NULL, argv[1],
                       "Your second argument in argv is incorrect!");

  return CG_TEST_PASSED;
}

static int
test_generate_exec_args_echo(void)
{
  int argc;
  char *argv[MAX_SHELL_ARGS];
  char cmd[32];

  snprintf(cmd, 32, "echo hello");
  argc = generate_exec_args(cmd, argv);
  CG_ASSERT_INT_EQ_MSG(2, argc, "Your argv array contains %d elements!", argc);

  CG_ASSERT_STR_EQ_MSG("echo", argv[0],
                       "Your first argument in argv is %s, it is incorrect",
                       argv[0]);
  CG_ASSERT_STR_EQ_MSG("hello", argv[1],
                       "Your second argument in argv is %s, it is incorrect!",
                       argv[1]);
  CG_ASSERT_PTR_EQ_MSG(NULL, argv[2],
                       "Your third argument in argv is %s, it is incorrect!",
                       argv[2]);
  return CG_TEST_PASSED;
}

static int
test_generate_exec_args_long(void)
{
  int argc;
  char *argv[MAX_SHELL_ARGS];
  char *ans[] = {"alpha",   "bravo", "charlie", "delta",   "echo",
                 "foxtrot", "golf",  "hotel",   "india",   "juliet",
                 "kilo",    "lima",  "mike",    "november"};
  int len     = sizeof ans / sizeof(char *);
  char cmd[256];
  snprintf(cmd, 255,
           "alpha bravo charlie delta echo foxtrot golf hotel india "
           "juliet kilo lima mike november");

  argc = generate_exec_args(cmd, argv);
  CG_ASSERT_INT_EQ(len, argc);
  for(int i = 0; i < len; i++) {
    CG_ASSERT_STR_EQ_MSG(ans[i], argv[i],
                         "Checking argument %d for argv, %s does not match %s",
                         i, argv[i], ans[i]);
  }
  CG_ASSERT_PTR_EQ_MSG(NULL, argv[len], "Last argument of argv is not correct");

  return CG_TEST_PASSED;
}

static struct cg_test_suite *
shell_test_suite(void)
{
  struct cg_test_suite *ts = cg_test_suite_new("Shell Tests", 0);
  if(!ts)
    exit(EXIT_FAILURE);

  CG_SUITE_CREATE_GRADED_TEST(ts, "test_generate_exec_args_ls",
                              test_generate_exec_args_ls, 5,
                              "Test generate_exec_args for the command 'ls'.");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_generate_exec_args_echo", test_generate_exec_args_echo, 5,
      "Test generate_exec_args for the command 'echo hello'");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_generate_exec_args_long",
                              test_generate_exec_args_long, 10,
                              "Test generate exec args for a long command");
  return ts;
}

int
main(int argc, char **argv)
{
#ifdef BUILD_GRADESCOPE
  struct cg_test_suite *ts = rf_load_test_suite();
  int rc                   = cg_test_suite_runall(ts);

  cg_test_suite_summarize(ts);
  cg_test_suite_gradescopify_tests(ts, "shell_tests.run.json");

  cg_test_suite_remove(ts);
  return (rc > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
#else
  int rc                  = 0;
  struct cg_project *proj = cg_project_new("SHELL TESTS");
  cg_project_add_suite(proj, shell_test_suite());

  cg_project_runall(proj);
  cg_project_summarize(proj);

  rc = proj->num_failures > 0 ? 1 : 0;
  cg_project_del(proj);
  return rc;
#endif
}
