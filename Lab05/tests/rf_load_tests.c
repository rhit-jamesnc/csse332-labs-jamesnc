/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * RF Loader Module Tests - 20 Points
 *
 * @author CGrade Test Suite
 * @date   2025-01-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cgassert.h"
#include "cgproject.h"
#include "cgrade.h"
#include "rf.h"
#include "rf_load.h"
#include "rf_parse.h"

// The list of runner format files for testing.
static const char *rfs[]    = {"1.rf", "2.rf", "3.rf", "4.rf", "5.rf", "6.rf"};
static int npages_needed[]  = {1, 1, 2, 2, 4, 3};
static int cpages_needed[]  = {1, 1, 1, 1, 1, 2};
static int effective_data[] = {0, 0, 4, 44, 9216, 4};
static int return_vals[]    = {5, 5, 105, 105, 294, 983076};
#define RFS_SIZE sizeof(rfs) / sizeof(const char *)

const char *
get_map_region(void *addr, int *np)
{
  static char mode[5] = {0};
  char buff[256];
  char *line = 0;
  size_t len;
  int rc;
  FILE *fp;
  unsigned long start, end;

  snprintf(buff, 256, "/proc/%d/maps", getpid());
  if(!(fp = fopen(buff, "r"))) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  while(getline(&line, &len, fp) > 0) {
    rc = sscanf(line, "%lx-%lx %s ", &start, &end, mode);
    if(rc < 3) {
      free(line);
      continue;
    }

    if((void *)start <= addr && addr < (void *)end) {
      // found it, return mode
      free(line);
      *np = (end - start) / getpagesize();
      return mode;
    }
  }
  if(line)
    free(line);
  return 0;
}

int
test_calc_npages(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  int npages = 0;

  for(int i = 0; i < RFS_SIZE; i++) {
    rf_parse_state_init(&st, rfs[i]);
    rf_read_hdr(&st, &hdr);
    CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                         "Header validation failed on %s", rfs[i]);
    npages = calc_npages(&st);
    CG_ASSERT_INT_EQ_MSG(npages_needed[i], npages, "Failed when running on %s!",
                         rfs[i]);
    rf_parse_state_destroy(&st);
  }

  return CG_TEST_PASSED;
}

int
test_load_code_exists(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  const char *m;
  void *code = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "1.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");

  code = rf_load_code(&st, code, &len);
  // 1. Check that the region exists
  CG_ASSERT_PTR_NNUL_MSG(code, "rf_load_code returned a null region");

  // 2. Check that the page is mapped in the page table
  m = get_map_region(code, &np);
  CG_ASSERT_PTR_NNUL_MSG(
      m, "Code region %p does not exist in the memory map!\n", code);

  rf_parse_state_destroy(&st);
  munmap(code, len);
  return CG_TEST_PASSED;
}

int
test_load_code_perm(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  const char *m;
  void *code = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "1.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");

  code = rf_load_code(&st, code, &len);
  // 3. Check the permissions on the code page.
  m    = get_map_region(code, &np);
  CG_ASSERT_STR_EQ_MSG(
      "r-xp", m, "Code region should be readable and executable only.!\n");

  rf_parse_state_destroy(&st);
  munmap(code, len);
  return CG_TEST_PASSED;
}

int
test_load_code_len(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  struct rf_shdr shdr;
  void *code = 0;
  int len    = 0;

  rf_parse_state_init(&st, "1.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");
  rf_read_section_header(&st, &shdr);

  code = rf_load_code(&st, code, &len);
  // 4. Check the returned length on the code page.
  CG_ASSERT_INT_EQ_MSG(
      shdr.len, len,
      "Code length returned from rf_load is not the same as the RF file.");

  rf_parse_state_destroy(&st);
  munmap(code, len);
  return CG_TEST_PASSED;
}

int
test_load_code_npages(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  struct rf_shdr shdr;
  void *code = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "6.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");
  rf_read_section_header(&st, &shdr);

  code = rf_load_code(&st, code, &len);
  get_map_region(code, &np);
  CG_ASSERT_INT_EQ_MSG(
      2, np, "Code area occupies %p pages while we can fit it in %p pages.", np,
      2);

  rf_parse_state_destroy(&st);
  munmap(code, len);
  return CG_TEST_PASSED;
}

int
test_load_code_hint(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  struct rf_shdr shdr;
  void *code = 0, *orig = 0;
  int len = 0;

  rf_parse_state_init(&st, "1.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");
  rf_read_section_header(&st, &shdr);

  code = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  orig = code;
  code = rf_load_code(&st, code, &len);
  CG_ASSERT_PTR_EQ_MSG(
      orig, code, "Code area is map incorrectly when given the hint %p", orig);

  rf_parse_state_destroy(&st);
  munmap(code, len);
  return CG_TEST_PASSED;
}

int
test_load_code_all(void)
{
  for(int i = 0; i < RFS_SIZE; i++) {
    struct rf_parse_state st = {0};
    struct rf_hdr hdr        = {0};
    struct rf_shdr shdr      = {0};
    const char *m;
    int np;
    void *code = 0, *rcode = 0;
    int len = 0;

    rf_parse_state_init(&st, rfs[i]);
    rf_read_hdr(&st, &hdr);
    CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                         "Header validation failed on %s", rfs[i]);
    rf_read_section_header(&st, &shdr);

    code  = mmap(NULL, cpages_needed[i] * getpagesize(), PROT_NONE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    rcode = rf_load_code(&st, code, &len);
    m     = get_map_region(rcode, &np);

    CG_ASSERT_PTR_NNUL_MSG(
        m, "[%s]: Code region %p does not exist in the memory map!\n", rfs[i],
        rcode);
    CG_ASSERT_STR_EQ_MSG(
        "r-xp", m,
        "[%s]: Code region should be readable and executable only.!\n", rfs[i]);
    CG_ASSERT_INT_EQ_MSG(shdr.len, len,
                         "[%s]: Code length returned from rf_load is not the "
                         "same as the RF file.",
                         rfs[i]);
    CG_ASSERT_INT_EQ_MSG(
        cpages_needed[i], np,
        "[%s]: Code area occupies %d pages while we can fit it in %d pages.",
        rfs[i], np, cpages_needed[i]);
    CG_ASSERT_PTR_EQ_MSG(
        code, rcode,
        "[%s]: Code area is map incorrectly when given the hint %p", rfs[i],
        code);

    munmap(code, cpages_needed[i] * getpagesize());
    rf_parse_state_destroy(&st);
  }

  return CG_TEST_PASSED;
}

int
test_load_no_data(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  void *data = 0;
  int len    = 0;

  rf_parse_state_init(&st, "1.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "1.rf");

  data = rf_load_data(&st, data, &len);
  CG_ASSERT_PTR_EQ_MSG(
      NULL, data, "1.rf does not contain a data region, but you returned %p",
      data);
  CG_ASSERT_INT_EQ_MSG(
      0, len, "1.rf does not contain a data, but your returned a length of %d",
      len);
  rf_parse_state_destroy(&st);
  return CG_TEST_PASSED;
}

int
test_load_data_exists(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  const char *m;
  void *code = 0;
  void *data = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "3.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "3.rf");

  // create a region to hold the data, one page below
  code =
      mmap(0, 2 * getpagesize(), PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  data = rf_load_data(&st, code, &len);
  // 1. Check that the region exists
  CG_ASSERT_PTR_NNUL_MSG(data, "rf_load_data returned a null region");

  // 2. Check that the page is mapped in the page table
  m = get_map_region(data, &np);
  CG_ASSERT_PTR_NNUL_MSG(
      m, "Data region %p does not exist in the memory map!\n", data);

  rf_parse_state_destroy(&st);
  munmap(code, 2 * getpagesize());
  return CG_TEST_PASSED;
}

int
test_load_data_perm(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  const char *m;
  void *code = 0;
  void *data = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "3.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "3.rf");

  // create a region to hold the data, one page below
  code =
      mmap(0, 2 * getpagesize(), PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  data = rf_load_data(&st, code, &len);
  // 3. Check the permissions on the data page.
  m    = get_map_region(data, &np);
  CG_ASSERT_STR_EQ_MSG("rw-p", m,
                       "Data region should be readable and writable only.!\n");

  rf_parse_state_destroy(&st);
  munmap(code, 2 * getpagesize());
  return CG_TEST_PASSED;
}

int
test_load_data_location(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  void *code = 0;
  void *data = 0;
  int len    = 0;

  rf_parse_state_init(&st, "3.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "3.rf");

  // create a region to hold the data, one page below
  code =
      mmap(0, 2 * getpagesize(), PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  data = rf_load_data(&st, code, &len);
  CG_ASSERT_PTR_EQ_MSG(code + getpagesize(), data,
                       "Data region (%p) is not mapped at the correct offset "
                       "relative to code (%p) on 3.rf",
                       data, code);

  munmap(code, 2 * getpagesize());
  rf_parse_state_destroy(&st);
  return CG_TEST_PASSED;
}

int
test_load_data_values(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  void *code = 0;
  void *data = 0;
  int len    = 0;
  int var    = 0;

  rf_parse_state_init(&st, "3.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "3.rf");

  // create a region to hold the data, one page below
  code =
      mmap(0, 2 * getpagesize(), PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  data = rf_load_data(&st, code, &len);
  var  = *(int *)data;
  CG_ASSERT_INT_EQ_MSG(
      100, var, "Data region (%p) does not contain correct variables", data);

  munmap(code, 2 * getpagesize());
  rf_parse_state_destroy(&st);
  return CG_TEST_PASSED;
}

int
test_load_data_len(void)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  void *code = 0;
  void *data = 0;
  int len    = 0;
  int np     = 0;

  rf_parse_state_init(&st, "3.rf");
  rf_read_hdr(&st, &hdr);
  CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                       "Header validation failed on %s", "3.rf");

  // create a region to hold the data, one page below
  code =
      mmap(0, 2 * getpagesize(), PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  data = rf_load_data(&st, code, &len);
  get_map_region(data, &np);
  CG_ASSERT_INT_EQ_MSG(1, np, "Data region mapped into %d pages instead of 1.",
                       np);
  CG_ASSERT_INT_EQ_MSG(4, len,
                       "Data region effective length was %d instead of 4", len);

  munmap(code, 2 * getpagesize());
  rf_parse_state_destroy(&st);
  return CG_TEST_PASSED;
}

static void
check_data_content(int i, void *data)
{
  // effecive is the name of the .c in proc/
  int effective = i + 1;

  if(effective == 3) {
    // already done, just return
    return;
  } else if(effective == 4) {
    // check for 4.c
    int *p = (int *)data;
    for(int i = 0; i < 10; i++) {
      CG_ASSERT_INT_EQ_MSG(
          -1, p[i], "4.rf initial content of the data region are incorect");
    }
    int *global = data + 10 * sizeof(int);
    CG_ASSERT_INT_EQ_MSG(
        100, *global, "4.rf initial content of the data region are incorrect");
  } else if(effective == 5) {
    // check for 5.c
    const char *p1 = data;
    for(int i = 0; i < 4096; i++) {
      if(i == 0) {
        CG_ASSERT_MSG(p1[i] == 'a',
                      "5.rf initial content of the data region are incorrect");
      } else {
        CG_ASSERT_MSG(p1[i] == 0,
                      "5.rf initial content of the data region are incorrect");
      }
    }
    p1 = data + 4096;
    for(int i = 0; i < 4096; i++) {
      if(i == 0) {
        CG_ASSERT_MSG(p1[i] == 'b',
                      "5.rf initial content of the data region are incorrect");
      } else {
        CG_ASSERT_MSG(p1[i] == 0,
                      "5.rf initial content of the data region are incorrect");
      }
    }
    p1 = data + 4096 + 4096;
    for(int i = 0; i < 1024; i++) {
      if(i == 0) {
        CG_ASSERT_MSG(p1[i] == 'c',
                      "5.rf initial content of the data region are incorrect");
      } else {
        CG_ASSERT_MSG(p1[i] == 0,
                      "5.rf initial content of the data region are incorrect");
      }
    }
  } else if(effective == 6) {
    int *sink = (int *)data;
    CG_ASSERT_INT_EQ_MSG(
        3, *sink, "6.rf initial content of the data region are incorect");
  }
}

int
test_load_data_all(void)
{
  for(int i = 0; i < RFS_SIZE; i++) {
    struct rf_parse_state st = {0};
    struct rf_hdr hdr        = {0};
    const char *m;
    int np;
    void *data = 0;
    void *code = 0;
    int len    = 0;
    int diff   = npages_needed[i] - cpages_needed[i];

    rf_parse_state_init(&st, rfs[i]);
    rf_read_hdr(&st, &hdr);
    CG_ASSERT_INT_EQ_MSG(0, rf_validate_hdr(&hdr, &st),
                         "Header validation failed on %s", rfs[i]);

    code = mmap(NULL, npages_needed[i] * getpagesize(), PROT_NONE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    data = rf_load_data(&st, code, &len);
    if(diff) {
      m = get_map_region(data, &np);

      CG_ASSERT_PTR_NNUL_MSG(
          m, "[%s]: Data region %p does not exist in the memory map!\n", rfs[i],
          data);
      CG_ASSERT_STR_EQ_MSG(
          "rw-p", m,
          "[%s]: Data region should be readable and executable only.!\n",
          rfs[i]);
      CG_ASSERT_INT_EQ_MSG(effective_data[i], len,
                           "[%s]: Code length returned from rf_load is not the "
                           "same as the RF file.",
                           rfs[i]);
      CG_ASSERT_INT_EQ_MSG(
          npages_needed[i] - cpages_needed[i], np,
          "[%s]: Code area occupies %d pages while we can fit it in %d pages.",
          rfs[i], np, npages_needed[i] - cpages_needed[i]);
      CG_ASSERT_PTR_EQ_MSG(code + cpages_needed[i] * getpagesize(), data,
                           "[%s]: Data region (%p) is mapped incorrectly at "
                           "the wrong offset from code %p",
                           rfs[i], data, code);

      check_data_content(i, data);
    } else {
      // no data region, make sure nothing is mapped
      CG_ASSERT_PTR_EQ_MSG(
          NULL, data,
          "Data region for %s does not exists but you mapped it to %p", rfs[i],
          data);
      CG_ASSERT_INT_EQ_MSG(0, len,
                           "Data region for %s does not exists but you "
                           "returned %d effective length",
                           rfs[i], len);
    }

    munmap(code, npages_needed[i] * getpagesize());
    rf_parse_state_destroy(&st);
  }

  return CG_TEST_PASSED;
}

int
test_exec_code_1rf(void)
{
  int rc;
  struct rf_exec_state exst = {0};
  const char *m;
  int npages;
  const char *target = "1.rf";

  rc = rf_exec_code_only(target, &exst);
  CG_ASSERT_INT_EQ_MSG(
      0, rc, "Executing the process from %s failed with error code %d", target,
      rc);

  // check the return value
  CG_ASSERT_INT_EQ_MSG(
      5, exst.rv,
      "Executing the process from %s return an incorrect return value %d",
      exst.rv);

  // Make sure memory regions exists and are of the right size
  m = get_map_region(exst.code, &npages);
  CG_ASSERT_STR_EQ_MSG(
      "r-xp", m, "Code region from %s does not exist or has wrong permissions",
      target);
  CG_ASSERT_INT_EQ_MSG(
      1, npages,
      "Code region from %s did not allocate the right number of pages", target);
  CG_ASSERT_PTR_EQ_MSG(NULL, exst.data,
                       "Data region for %s should be null but is not", target);
  CG_ASSERT_INT_EQ_MSG(0, exst.dlen,
                       "Data length for %s should be 0 but is not", target);

  rf_unmap_state(&exst);
  return CG_TEST_PASSED;
}

int
test_exec_code_2rf(void)
{
  int rc;
  struct rf_exec_state exst = {0};
  const char *m;
  int npages;
  const char *target = "2.rf";

  rc = rf_exec_code_only(target, &exst);
  CG_ASSERT_INT_EQ_MSG(
      0, rc, "Executing the process from %s failed with error code %d", target,
      rc);

  // check the return value
  CG_ASSERT_INT_EQ_MSG(
      5, exst.rv,
      "Executing the process from %s return an incorrect return value %d",
      exst.rv);

  // Make sure memory regions exists and are of the right size
  m = get_map_region(exst.code, &npages);
  CG_ASSERT_STR_EQ_MSG(
      "r-xp", m, "Code region from %s does not exist or has wrong permissions",
      target);
  CG_ASSERT_INT_EQ_MSG(
      1, npages,
      "Code region from %s did not allocate the right number of pages", target);
  CG_ASSERT_PTR_EQ_MSG(NULL, exst.data,
                       "Data region for %s should be null but is not", target);
  CG_ASSERT_INT_EQ_MSG(0, exst.dlen,
                       "Data length for %s should be 0 but is not", target);

  rf_unmap_state(&exst);
  return CG_TEST_PASSED;
}

int
test_exec_code_7rf(void)
{
  int rc;
  struct rf_exec_state exst = {0};
  const char *m;
  int npages;
  const char *target = "7.rf";

  rc = rf_exec_code_only(target, &exst);
  CG_ASSERT_INT_EQ_MSG(
      0, rc, "Executing the process from %s failed with error code %d", target,
      rc);

  // check the return value
  CG_ASSERT_INT_EQ_MSG(
      983076, exst.rv,
      "Executing the process from %s return an incorrect return value %d",
      exst.rv);

  // Make sure memory regions exists and are of the right size
  m = get_map_region(exst.code, &npages);
  CG_ASSERT_STR_EQ_MSG(
      "r-xp", m, "Code region from %s does not exist or has wrong permissions",
      target);
  CG_ASSERT_INT_EQ_MSG(
      2, npages,
      "Code region from %s did not allocate the right number of pages", target);
  CG_ASSERT_PTR_EQ_MSG(NULL, exst.data,
                       "Data region for %s should be null but is not", target);
  CG_ASSERT_INT_EQ_MSG(0, exst.dlen,
                       "Data length for %s should be 0 but is not", target);

  rf_unmap_state(&exst);
  return CG_TEST_PASSED;
}

int
test_exec(void)
{
  for(int i = 0; i < RFS_SIZE; i++) {
    int rc;
    struct rf_exec_state exst = {0};
    const char *m;
    int npages;

    rc = rf_exec(rfs[i], &exst);
    CG_ASSERT_INT_EQ_MSG(
        0, rc, "Executing the process from %s failed with error code %d",
        rfs[i], rc);

    // check the return value
    CG_ASSERT_INT_EQ_MSG(
        return_vals[i], exst.rv,
        "Executing the process from %s return an incorrect return value %d",
        exst.rv);

    // Make sure memory regions exists and are of the right size
    m = get_map_region(exst.code, &npages);
    CG_ASSERT_STR_EQ_MSG(
        "r-xp", m,
        "Code region from %s does not exist or has wrong permissions", rfs[i]);
    CG_ASSERT_INT_EQ_MSG(
        cpages_needed[i], npages,
        "Code region from %s did not allocate the right number of pages",
        rfs[i]);

    if(exst.data) {
      m = get_map_region(exst.data, &npages);
      CG_ASSERT_STR_EQ_MSG(
          "rw-p", m,
          "Data region from %s does not exist or has wrong permissions",
          rfs[i]);
      CG_ASSERT_INT_EQ_MSG(
          npages_needed[i] - cpages_needed[i], npages,
          "Data region from %s did not allocate the right number of pages",
          rfs[i]);
    }

    rf_unmap_state(&exst);
  }
  return CG_TEST_PASSED;
}

static struct cg_test_suite *
rf_load_test_suite(void)
{
  struct cg_test_suite *ts = cg_test_suite_new("RF Loader Tests", 0);
  if(!ts)
    exit(EXIT_FAILURE);

  // calc_npages: 10 points
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_calc_npages", test_calc_npages, 10,
      "Calculate the number of pages needed for each binary");

  // Basic load code tests
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_load_code_exists",
                              test_load_code_exists, 4,
                              "Test that loaded code exists in memory map");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_code_perm", test_load_code_perm, 8,
      "Test that loaded code region has the right permissions");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_code_len", test_load_code_len, 4,
      "Test that loaded code region has the correct length");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_code_npages", test_load_code_npages, 8,
      "Test that loaded code regions fits within the required number of pages");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_code_hint", test_load_code_hint, 4,
      "Test that loading the code region is consistent when given a hint");

  // Test loading code for all examples
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_code_all", test_load_code_all, 10,
      "Test that loading code works for all of the examples we have.");

  // Test exec for code-only examples
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_exec_code_1rf", test_exec_code_1rf, 5,
      "Test that executing code-only example (1.rf) works correctly");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_exec_code_2rf", test_exec_code_2rf, 5,
      "Test that executing code-only example (2.rf) works correctly");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_exec_code_7rf", test_exec_code_7rf, 5,
      "Test that executing code-only example (7.rf) works correctly");

  // Basic loading of data region
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_no_data", test_load_no_data, 5,
      "Test that programs with no data get nothing loaded");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_data_exists", test_load_data_exists, 4,
      "Test that loaded data region exists in memory map");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_data_perm", test_load_data_perm, 8,
      "Test that loaded data region has the right permissions");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_load_data_location",
                              test_load_data_location, 8,
                              "Test that loaded data is at the right offset");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_data_values", test_load_data_values, 4,
      "Test that loaded data region contains the right variable");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_data_len", test_load_data_len, 4,
      "Test that loaded data region is of correct length");

  // Test loading data for all examples
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_load_data_all", test_load_data_all, 24,
      "Test that loading data works for all the examples we have");

  // Test exec that integrates everything.
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_exec", test_exec, 30,
      "Integration tests using rf_exec to make sure code executes correctly");

  return ts;
}

int
main(int argc, char **argv)
{
#ifdef BUILD_GRADESCOPE
  struct cg_test_suite *ts = rf_load_test_suite();
  int rc                   = cg_test_suite_runall(ts);

  cg_test_suite_summarize(ts);
  cg_test_suite_gradescopify_tests(ts, "rf_load_tests.run.json");

  cg_test_suite_remove(ts);
  return (rc > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
#else
  int rc                  = 0;
  struct cg_project *proj = cg_project_new("RF Loader");
  cg_project_add_suite(proj, rf_load_test_suite());

  cg_project_runall(proj);
  cg_project_summarize(proj);

  rc = proj->num_failures > 0 ? 1 : 0;
  cg_project_del(proj);
  return rc;
#endif
}
