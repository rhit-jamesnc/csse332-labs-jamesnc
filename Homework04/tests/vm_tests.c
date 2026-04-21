
/**
 * Copyright (c) 2026 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */

#include <unistd.h>

#include "cgrade.h"
#include "cgradescropify.h"
#include "vm.h"

static int
count_pages_in_state(unsigned char state)
{
  struct pte *pt = get_pgtbl();
  int n          = 0;

  for(int i = 0; i < NUM_VIRT_PAGES; i++)
    if(pt[i].state == state)
      n++;
  return n;
}

/* Write 0 to the first int of page pg, triggering a fault if not yet mapped. */
static void
touch_page(void *base, int pg)
{
  ((int *)base)[pg * (getpagesize() / sizeof(int))] = 0;
}

/*****************************************************************************
 * Part 1 — Lazy allocation
 *****************************************************************************/

static int
test_init_sets_memstart(void)
{
  init_vm();
  CG_ASSERT_MSG(get_memstart() != 0,
                "get_memstart() should be non-zero after init_vm");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_init_pagetable_all_unused(void)
{
  get_pgtbl()[0].state = ACTIVE;
  init_vm();
  CG_ASSERT_INT_EQ_MSG(NUM_VIRT_PAGES, count_pages_in_state(UNUSED),
                       "All %d page table entries must be UNUSED after init_vm",
                       NUM_VIRT_PAGES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_give_me_pages_one(void)
{
  void *addr;
  init_vm();
  addr = give_me_pages(1);
  CG_ASSERT_PTR_NNUL_MSG(addr, "give_me_pages(1) should return non-NULL");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_give_me_pages_max(void)
{
  void *addr;
  init_vm();
  addr = give_me_pages(NUM_VIRT_PAGES);
  CG_ASSERT_PTR_NNUL_MSG(
      addr, "give_me_pages(%d) (exact maximum) should return non-NULL",
      NUM_VIRT_PAGES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_give_me_pages_too_many(void)
{
  void *addr;
  init_vm();
  addr = give_me_pages(NUM_VIRT_PAGES + 1);
  CG_ASSERT_MSG(addr == 0,
                "give_me_pages(%d) should return 0 (exceeds NUM_VIRT_PAGES)",
                NUM_VIRT_PAGES + 1);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_give_me_pages_matches_memstart(void)
{
  void *addr;
  init_vm();
  addr = give_me_pages(NUM_VIRT_PAGES);
  CG_ASSERT_PTR_EQ_MSG(
      (void *)get_memstart(), addr,
      "give_me_pages should return the VM region base address");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_page_unused_before_access(void)
{
  // This will be a false positive at the start, but will adjusted once we add
  // more tests.
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  (void)mem; /* do not touch any page */
  CG_ASSERT_INT_EQ_MSG(
      UNUSED, pt[0].state,
      "Page 0 must be UNUSED before its first access, it is %s instead",
      pg_state_str(pt[0].state));
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_page_active_after_write(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem             = give_me_pages(NUM_VIRT_PAGES);
  pt              = get_pgtbl();
  ((int *)mem)[0] = 1; /* triggers SIGSEGV handler */
  CG_ASSERT_INT_EQ_MSG(
      ACTIVE, pt[0].state,
      "Page 0 must be ACTIVE after its first write, it is %s instead",
      pg_state_str(pt[0].state));
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_page_active_after_read(void)
{
  volatile int val;
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  val = ((int *)mem)[0]; /* triggers SIGSEGV handler on read */
  (void)val;
  CG_ASSERT_INT_EQ_MSG(
      ACTIVE, pt[0].state,
      "Page 0 must be ACTIVE after its first read, it is %s instead",
      pg_state_str(pt[0].state));
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_freshness_set_after_map(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem             = give_me_pages(NUM_VIRT_PAGES);
  pt              = get_pgtbl();
  ((int *)mem)[0] = 1;
  CG_ASSERT_INT_EQ_MSG(
      NUM_PHYS_FRAMES, (int)pt[0].freshness,
      "Freshness must be NUM_PHYS_FRAMES (%d) right after mapping",
      NUM_PHYS_FRAMES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_page_offset_after_map(void)
{
  void *mem;
  struct pte *pt;
  int pgsz   = getpagesize();
  int stride = pgsz / sizeof(int);
  init_vm();
  mem                      = give_me_pages(NUM_VIRT_PAGES);
  pt                       = get_pgtbl();
  ((int *)mem)[0]          = 1; /* map page 0 */
  ((int *)mem)[2 * stride] = 1; /* map page 2 */
  CG_ASSERT_LONG_EQ_MSG(0L, (long)pt[0].offset, "Page 0 swap offset must be 0");
  CG_ASSERT_LONG_EQ_MSG((long)(2 * pgsz), (long)pt[2].offset,
                        "Page 2 swap offset must be 2 * pagesize");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_data_persists_in_page(void)
{
  void *mem;
  int i;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(i = 0; i < 8; i++)
    ((int *)mem)[i] = i * 10;
  for(i = 0; i < 8; i++)
    CG_ASSERT_INT_EQ_MSG(
        i * 10, ((int *)mem)[i],
        "((int*)mem)[%d] should still hold %d after earlier write", i, i * 10);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_write_full_page(void)
{
  void *mem;
  int n = getpagesize() / sizeof(int);
  int i;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(i = 0; i < n; i++)
    ((int *)mem)[i] = i + 1;
  for(i = 0; i < n; i++)
    CG_ASSERT_INT_EQ_MSG(
        i + 1, ((int *)mem)[i],
        "((int*)mem)[%d] should be %d after filling page 0 entirely", i, i + 1);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_untouched_pages_stay_unused(void)
{
  void *mem;
  struct pte *pt;
  int i;
  init_vm();
  mem             = give_me_pages(NUM_VIRT_PAGES);
  pt              = get_pgtbl();
  ((int *)mem)[0] = 1; /* only access page 0 */
  for(i = 1; i < NUM_VIRT_PAGES; i++)
    CG_ASSERT_INT_EQ_MSG(UNUSED, pt[i].state,
                         "Page %d should remain UNUSED (never accessed)", i);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_num_phys_frames_pages_no_eviction(void)
{
  void *mem;
  struct pte *pt;
  int stride = getpagesize() / sizeof(int);
  int pg;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  /* Touch exactly NUM_PHYS_FRAMES pages — eviction must NOT trigger */
  for(pg = 0; pg < NUM_PHYS_FRAMES; pg++)
    ((int *)mem)[pg * stride] = pg;
  for(pg = 0; pg < NUM_PHYS_FRAMES; pg++)
    CG_ASSERT_INT_EQ_MSG(ACTIVE, pt[pg].state,
                         "Page %d should be ACTIVE after being accessed", pg);
  CG_ASSERT_INT_EQ_MSG(
      0, count_pages_in_state(EVICTED),
      "No page should be EVICTED when only %d pages are accessed",
      NUM_PHYS_FRAMES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_adjacent_pages_data_independent(void)
{
  void *mem;
  int stride = getpagesize() / sizeof(int);
  init_vm();
  mem                      = give_me_pages(NUM_VIRT_PAGES);
  ((int *)mem)[0]          = 111;
  ((int *)mem)[stride]     = 222;
  ((int *)mem)[2 * stride] = 333;
  CG_ASSERT_INT_EQ_MSG(111, ((int *)mem)[0], "Page 0 data should be 111");
  CG_ASSERT_INT_EQ_MSG(222, ((int *)mem)[stride], "Page 1 data should be 222");
  CG_ASSERT_INT_EQ_MSG(333, ((int *)mem)[2 * stride],
                       "Page 2 data should be 333");
  teardown_vm();
  return CG_TEST_PASSED;
}

/*****************************************************************************
 * Part 2 — Eviction
 * (eviction triggers, state transitions, data persistence, edge cases)
 *****************************************************************************/

static int
test_fourth_page_triggers_eviction(void)
{
  void *mem;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  CG_ASSERT_INT_EQ_MSG(1, count_pages_in_state(EVICTED),
                       "Accessing %d pages should evict exactly 1 page",
                       NUM_PHYS_FRAMES + 1);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_active_count_after_fourth_access(void)
{
  void *mem;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  CG_ASSERT_INT_EQ_MSG(NUM_PHYS_FRAMES, count_pages_in_state(ACTIVE),
                       "Exactly NUM_PHYS_FRAMES (%d) pages should be ACTIVE "
                       "after the first eviction",
                       NUM_PHYS_FRAMES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_correct_page_evicted(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  /* access 0, 1, 2, 3 in order → page 0 freshness hits 0 first */
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  CG_ASSERT_INT_EQ_MSG(EVICTED, pt[0].state,
                       "Page 0 should be EVICTED when pages 0..%d are accessed "
                       "in order",
                       NUM_PHYS_FRAMES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_evicted_page_freshness_zero(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  CG_ASSERT_INT_EQ_MSG(0, (int)pt[0].freshness,
                       "An evicted page must have freshness 0");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_active_never_exceeds_frames(void)
{
  void *mem;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++) {
    touch_page(mem, pg);
    CG_ASSERT_MSG(count_pages_in_state(ACTIVE) <= NUM_PHYS_FRAMES,
                  "Active page count exceeded NUM_PHYS_FRAMES (%d) after "
                  "accessing page %d",
                  NUM_PHYS_FRAMES, pg);
  }
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_data_survives_eviction(void)
{
  void *mem;
  int sentinel = 12345;
  init_vm();
  mem             = give_me_pages(NUM_VIRT_PAGES);
  ((int *)mem)[0] = sentinel; /* write to page 0 */
  /* access pages 1..NUM_PHYS_FRAMES to evict page 0 */
  for(int pg = 1; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  /* re-read page 0 — fault reloads it from swap */
  CG_ASSERT_INT_EQ_MSG(sentinel, ((int *)mem)[0],
                       "Sentinel written to page 0 should survive eviction "
                       "and reload from swap");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_evicted_page_becomes_active_on_reaccess(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg); /* evicts page 0 */
  CG_ASSERT_INT_EQ(EVICTED, pt[0].state);
  touch_page(mem, 0); /* fault page 0 back in */
  CG_ASSERT_INT_EQ_MSG(
      ACTIVE, pt[0].state,
      "Re-accessing an EVICTED page should restore it to ACTIVE");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_reloaded_page_freshness_reset(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  for(int pg = 0; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  touch_page(mem, 0); /* reload page 0 */
  CG_ASSERT_INT_EQ_MSG(NUM_PHYS_FRAMES, (int)pt[0].freshness,
                       "Freshness must be reset to NUM_PHYS_FRAMES when an "
                       "evicted page is reloaded");
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_full_page_survives_eviction(void)
{
  void *mem;
  int n = getpagesize() / sizeof(int);
  int i;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  /* fill page 0 completely */
  for(i = 0; i < n; i++)
    ((int *)mem)[i] = i + 1;
  /* evict page 0 */
  for(int pg = 1; pg <= NUM_PHYS_FRAMES; pg++)
    touch_page(mem, pg);
  /* reload and verify every integer */
  for(i = 0; i < n; i++)
    CG_ASSERT_INT_EQ_MSG(
        i + 1, ((int *)mem)[i],
        "((int*)mem)[%d] on page 0 should be %d after evict/reload", i, i + 1);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_all_pages_data_intact(void)
{
  void *mem;
  int stride = getpagesize() / sizeof(int);
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  /* write a distinct value to the first int of every page */
  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++)
    ((int *)mem)[pg * stride] = pg * 100 + 7;
  /* read every page back — evicted pages fault in from swap */
  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++)
    CG_ASSERT_INT_EQ_MSG(
        pg * 100 + 7, ((int *)mem)[pg * stride],
        "Page %d first-int should be %d after eviction cycling", pg,
        pg * 100 + 7);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_no_unused_after_full_scan(void)
{
  void *mem;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++)
    touch_page(mem, pg);
  CG_ASSERT_INT_EQ_MSG(0, count_pages_in_state(UNUSED),
                       "No page should be UNUSED after all %d pages have been "
                       "accessed",
                       NUM_VIRT_PAGES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_last_frames_active_after_full_scan(void)
{
  void *mem;
  struct pte *pt;
  init_vm();
  mem = give_me_pages(NUM_VIRT_PAGES);
  pt  = get_pgtbl();
  for(int pg = 0; pg < NUM_VIRT_PAGES; pg++)
    touch_page(mem, pg);
  /* the last NUM_PHYS_FRAMES pages accessed must still be ACTIVE */
  for(int pg = NUM_VIRT_PAGES - NUM_PHYS_FRAMES; pg < NUM_VIRT_PAGES; pg++)
    CG_ASSERT_INT_EQ_MSG(
        ACTIVE, pt[pg].state,
        "Page %d should be ACTIVE (one of the last %d accessed)", pg,
        NUM_PHYS_FRAMES);
  teardown_vm();
  return CG_TEST_PASSED;
}

static int
test_repeated_evict_reload_preserves_data(void)
{
  void *mem;
  struct pte *pt;
  int sentinel = 99;
  int round;
  init_vm();
  mem             = give_me_pages(NUM_VIRT_PAGES);
  pt              = get_pgtbl();
  ((int *)mem)[0] = sentinel;
  for(round = 0; round < 3; round++) {
    /* evict page 0 */
    for(int pg = 1; pg <= NUM_PHYS_FRAMES; pg++)
      touch_page(mem, pg);
    CG_ASSERT_MSG(
        pt[0].state == EVICTED,
        "Round %d: page 0 should be EVICTED after accessing pages 1-%d", round,
        NUM_PHYS_FRAMES);
    /* reload page 0 and verify sentinel */
    CG_ASSERT_INT_EQ_MSG(sentinel, ((int *)mem)[0],
                         "Round %d: sentinel should survive reload from swap",
                         round);
    CG_ASSERT_INT_EQ_MSG(ACTIVE, pt[0].state,
                         "Round %d: page 0 should be ACTIVE after reload",
                         round);
  }
  teardown_vm();
  return CG_TEST_PASSED;
}

struct cg_test_suite *
vm_test_suite(void)
{
  struct cg_test_suite *ts = cg_test_suite_new("VM subsystem tests", 0);

  /* ----- Part 1: lazy allocation ----- */
  CG_SUITE_CREATE_GRADED_TEST(ts, "init_vm sets mem_start",
                              test_init_sets_memstart, 3,
                              "get_memstart() must be non-zero after init_vm");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "page table all UNUSED after init", test_init_pagetable_all_unused, 4,
      "Every page table entry must start in the UNUSED state");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "give_me_pages(1) non-NULL", test_give_me_pages_one, 4,
      "give_me_pages(1) should return a non-NULL address");
  CG_SUITE_CREATE_GRADED_TEST(ts, "give_me_pages(NUM_VIRT_PAGES) succeeds",
                              test_give_me_pages_max, 4,
                              "give_me_pages(NUM_VIRT_PAGES) should succeed");
  CG_SUITE_CREATE_GRADED_TEST(ts, "give_me_pages(NUM_VIRT_PAGES+1) returns 0",
                              test_give_me_pages_too_many, 4,
                              "give_me_pages(NUM_VIRT_PAGES+1) must return 0");
  CG_SUITE_CREATE_GRADED_TEST(ts, "give_me_pages address equals mem_start",
                              test_give_me_pages_matches_memstart, 4,
                              "give_me_pages should return get_memstart()");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "page UNUSED before any access", test_page_unused_before_access, 4,
      "A page must be UNUSED until it is first touched");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "page ACTIVE after first write", test_page_active_after_write, 5,
      "Writing to a page must trigger the handler and set it ACTIVE");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "page ACTIVE after first read", test_page_active_after_read, 5,
      "Reading from a page for the first time must also set it ACTIVE");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "freshness is NUM_PHYS_FRAMES after mapping",
      test_freshness_set_after_map, 4,
      "Freshness must equal NUM_PHYS_FRAMES right after first map");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "swap offset correct after mapping", test_page_offset_after_map, 4,
      "Page N offset must be N * pagesize in the swap file");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "data persists in mapped page", test_data_persists_in_page, 5,
      "Values written to a page must be readable from the same addresses");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "write and read back entire page", test_write_full_page, 5,
      "All integers within a page must survive write-then-read");
  CG_SUITE_CREATE_GRADED_TEST(ts, "untouched pages remain UNUSED",
                              test_untouched_pages_stay_unused, 4,
                              "Pages that are never accessed must stay UNUSED");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "exactly NUM_PHYS_FRAMES pages: all ACTIVE, none evicted",
      test_num_phys_frames_pages_no_eviction, 5,
      "Accessing exactly NUM_PHYS_FRAMES pages must not trigger eviction");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "adjacent pages hold independent data",
      test_adjacent_pages_data_independent, 5,
      "Writes to pages 0, 1, and 2 must not interfere with each other");

  /* ----- Part 2: eviction ----- */
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "4th page access triggers exactly 1 eviction",
      test_fourth_page_triggers_eviction, 5,
      "Accessing NUM_PHYS_FRAMES+1 distinct pages should evict exactly one");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "active count is NUM_PHYS_FRAMES after eviction",
      test_active_count_after_fourth_access, 5,
      "After the first eviction there must be exactly NUM_PHYS_FRAMES "
      "active pages");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "correct page evicted (LRU order)", test_correct_page_evicted, 5,
      "When pages 0..NUM_PHYS_FRAMES are accessed in order, page 0 "
      "(least fresh) must be evicted");
  CG_SUITE_CREATE_GRADED_TEST(ts, "evicted page freshness is 0",
                              test_evicted_page_freshness_zero, 4,
                              "evict_page must set freshness to 0");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "active count never exceeds NUM_PHYS_FRAMES",
      test_active_never_exceeds_frames, 6,
      "At no point during a full sequential scan should active count "
      "exceed NUM_PHYS_FRAMES");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "sentinel survives eviction and reload", test_data_survives_eviction,
      6,
      "A value written to page 0 must be readable after eviction and "
      "reload from swap");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "evicted page becomes ACTIVE on re-access",
      test_evicted_page_becomes_active_on_reaccess, 5,
      "Re-accessing an EVICTED page must restore it to ACTIVE");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "reloaded page freshness reset to NUM_PHYS_FRAMES",
      test_reloaded_page_freshness_reset, 4,
      "Freshness must be NUM_PHYS_FRAMES when a page is reloaded from swap");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "full page contents survive eviction/reload",
      test_full_page_survives_eviction, 7,
      "All integers within a page must survive being evicted and reloaded");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "all-pages data intact after eviction cycling",
      test_all_pages_data_intact, 8,
      "Writing a distinct value to each of the 15 pages and reading "
      "them all back must preserve every value");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "no UNUSED pages after full scan", test_no_unused_after_full_scan, 4,
      "All pages must have been mapped at least once after accessing "
      "all NUM_VIRT_PAGES");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "last NUM_PHYS_FRAMES pages ACTIVE after full scan",
      test_last_frames_active_after_full_scan, 5,
      "After a full sequential scan the last NUM_PHYS_FRAMES pages "
      "must be ACTIVE");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "repeated evict/reload preserves data",
      test_repeated_evict_reload_preserves_data, 8,
      "A page can be evicted and reloaded multiple times without "
      "data corruption");

  return ts;
}

int
main(int argc, char **argv)
{
#ifdef BUILD_GRADESCOPE
  struct cg_test_suite *ts = vm_test_suite();
  int rc                   = cg_test_suite_runall(ts);

  cg_test_suite_summarize(ts);
  cg_test_suite_gradescopify_tests(ts, "vm_tests.run.json");

  cg_test_suite_remove(ts);
  rc = (ts->num_failures > 0) ? 1 : 0;
  return (rc > 0) ? EXIT_FAILURE : EXIT_SUCCESS;

#else
  int rc                  = 0;
  struct cg_project *proj = cg_project_new("Lazy Swapped Virtual Memory");
  cg_project_add_suite(proj, vm_test_suite());

  cg_project_runall(proj);
  cg_project_summarize(proj);

  rc = proj->num_failures > 0 ? 1 : 0;
  cg_project_del(proj);
  return rc;
#endif
}
