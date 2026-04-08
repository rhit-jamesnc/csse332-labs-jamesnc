/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/8/26
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "cgassert.h"
#include "cgproject.h"
#include "cgrade.h"
#include "cgradescropify.h"
#include "rhmalloc.h"

/* Simple helper macro */
#define HEAP_MEM_START heap_start()
#define META_SIZE sizeof(struct metadata)

/**
 * Check if we need to do a reset at the start of a test case.
 */
static void
_reset_if_needed(void)
{
  if(freelist_head())
    rhfree_all();
}

static int
test_basic_alloc(void)
{
  void *ptr              = 0;
  struct metadata *entry = 0;

  _reset_if_needed();

  // Try to simply allocate 80 bytes
  ptr = rhmalloc(80);
  CG_ASSERT_PTR_EQ_MSG(
      HEAP_MEM_START + META_SIZE, ptr,
      "Test that first allocation is always at head for free list");

  // Make sure the metadata has been set up correctly
  entry = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry);
  CG_ASSERT_INT_EQ_MSG(1, entry->in_use,
                       "The metadata entry should be marked in use");
  CG_ASSERT_INT_EQ_MSG(80, entry->size,
                       "The metadata entry does not have the correct size");
  CG_ASSERT_PTR_EQ_MSG(
      0, entry->prev,
      "The previous pointer of the entry in use should be NULL");

  // Make sure the first block has been split correctly
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + 80 + META_SIZE, entry->next);
  CG_ASSERT_INT_EQ(0, entry->next->in_use);
  CG_ASSERT_INT_EQ(MAX_HEAP_SIZE - 2 * META_SIZE - 80, entry->next->size);
  CG_ASSERT_PTR_EQ(0, entry->next->next);
  CG_ASSERT_PTR_EQ(entry, entry->next->prev);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_two_allocs(void)
{
  void *ptr                = 0;
  struct metadata *entry   = 0;
  struct metadata *nentry  = 0;
  struct metadata *nnentry = 0;

  _reset_if_needed();

  // Allocate twice.
  ptr    = rhmalloc(80);
  entry  = freelist_head();
  nentry = entry->next;

  ptr     = rhmalloc(160);
  nnentry = freelist_head()->next;

  // check that we have further split correctly
  CG_ASSERT_PTR_EQ_MSG(HEAP_MEM_START + 80 + 2 * META_SIZE, ptr,
                       "After two mallocs, blocks are not split correctly.");
  CG_ASSERT_PTR_EQ_MSG(
      nentry, nnentry,
      "The first entry should not have changed after a second malloc.");
  CG_ASSERT_INT_EQ(160, nnentry->size);
  CG_ASSERT_INT_EQ(1, nnentry->in_use);
  CG_ASSERT_PTR_EQ(entry, nnentry->prev);

  // check on the remaining free block
  entry = nnentry->next;
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + 160 + 80 + 2 * META_SIZE, entry);
  CG_ASSERT_INT_EQ(MAX_HEAP_SIZE - 3 * META_SIZE - 160 - 80, entry->size);
  CG_ASSERT_PTR_EQ_MSG(nnentry, entry->prev,
                       "Empty block should be at the end.");
  CG_ASSERT_PTR_EQ_MSG(0, entry->next,
                       "Empty block should not have a next pointer.");

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_alignment(void)
{
  void *blk1, *blk2;
  struct metadata *entry1, *entry2;

  _reset_if_needed();

  blk1   = rhmalloc(1);
  blk2   = rhmalloc(ALIGNMENT * 3 + 1);
  entry1 = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);

  /* It doesn't make sense to really allocate a single byte, we have to keep
   * some alignment restrictions. So when we have to force the actual memory
   * region to be different than the requested size, we should store the actual
   * size rather than the requested size. This is because the true size isn't
   * something the user will have access to. */
  CG_ASSERT_INT_EQ(ALIGNMENT, entry1->size);

  entry2 = entry1->next;
  CG_ASSERT_INT_EQ(4 * ALIGNMENT, entry2->size);
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + META_SIZE + ALIGNMENT, entry2);

  CG_ASSERT_PTR_EQ(HEAP_MEM_START + META_SIZE, blk1);
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + META_SIZE * 2 + ALIGNMENT, blk2);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_basic_free(void)
{
  void *ptr;
  struct metadata *entry;

  ptr   = rhmalloc(80);
  entry = freelist_head();
  rhmalloc(160);

  // free the memory and make sure we're back at the initial spot.
  rhfree(ptr);

  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry);
  CG_ASSERT_INT_EQ(80, entry->size);
  CG_ASSERT_INT_EQ(0, entry->in_use);
  CG_ASSERT_PTR_EQ(0, entry->prev);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_more_free(void)
{
  void *block1ptr;
  struct metadata *entry1, *block2_metadata, *reused_block, *little_remainder,
      *big_block_metadata;

  /* reset if needed */
  _reset_if_needed();

  /* Allocate some memory */
  block1ptr = rhmalloc(80);
  rhmalloc(160);

  /* free the first block */
  rhfree(block1ptr);

  entry1 = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);
  CG_ASSERT_INT_EQ(80, entry1->size);
  CG_ASSERT_INT_EQ(0, entry1->in_use);
  CG_ASSERT_PTR_EQ(0, entry1->prev);

  block2_metadata = entry1->next;
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + META_SIZE + 80, block2_metadata);

  /* Now allocate a block we can fit in the space we just freed. Given our
   * design, this block will go into the first free block which is the one that
   * we just freed. That is because we are keeping out block doubly linked list
   * in order by address. */
  reused_block = rhmalloc(40);
  CG_ASSERT_PTR_EQ(HEAP_MEM_START + META_SIZE, reused_block);
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);

  /* check that entry1 has been updated! */
  CG_ASSERT_INT_EQ(40, entry1->size);
  CG_ASSERT_INT_EQ(1, entry1->in_use);
  CG_ASSERT_PTR_EQ(0, entry1->prev);

  /* There should be a small free block after the newly allocated one and before
   * the one we had previosly allocated. */
  little_remainder = entry1->next;
  CG_ASSERT_PTR_EQ(entry1, little_remainder->prev);
  CG_ASSERT_PTR_EQ(block2_metadata, little_remainder->next);

  CG_ASSERT_INT_EQ(40 - META_SIZE, little_remainder->size);
  CG_ASSERT_INT_EQ(0, little_remainder->in_use);

  CG_ASSERT_PTR_EQ(little_remainder, block2_metadata->prev);

  /* make a new block that cannot fit into the small amount of remainder memory.
   */
  rhmalloc(88);
  CG_ASSERT_INT_EQ(0, little_remainder->in_use);

  /* since it can't fit there, it will go after the last block. */
  big_block_metadata = block2_metadata->next;
  CG_ASSERT_INT_EQ(1, big_block_metadata->in_use);
  CG_ASSERT_INT_EQ(88, big_block_metadata->size);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_coalesce_right(void)
{
  void *blk1, *blk2;
  struct metadata *entry1, *entry2;

  _reset_if_needed();

  blk1 = rhmalloc(80);
  blk2 = rhmalloc(88);
  rhmalloc(96);

  rhfree(blk2);

  /* now when we free blk1, blk2 should already have been free and thus should
   * be coalesced with its next block. */
  rhfree(blk1);

  entry1 = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);
  CG_ASSERT_INT_EQ(80 + 88 + META_SIZE, entry1->size);
  CG_ASSERT_INT_EQ(0, entry1->in_use);

  entry2 = freelist_head()->next;
  CG_ASSERT_INT_EQ(96, entry2->size);
  CG_ASSERT_INT_EQ(1, entry2->in_use);
  CG_ASSERT_PTR_EQ(entry1, entry2->prev);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_coalesce_left(void)
{
  void *blk1, *blk2;
  struct metadata *entry1, *entry2;

  _reset_if_needed();

  blk1 = rhmalloc(80);
  blk2 = rhmalloc(88);
  rhmalloc(96);

  /* exactly the same as coalesce basic 1, but now the order of the frees is
   * reversed. When blk2 is freed, blk1 is already freed and blk2 must coleasce
   * into blk1. */
  rhfree(blk1);
  rhfree(blk2);

  entry1 = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);
  CG_ASSERT_INT_EQ(80 + 88 + META_SIZE, entry1->size);
  CG_ASSERT_INT_EQ(0, entry1->in_use);

  entry2 = freelist_head()->next;
  CG_ASSERT_INT_EQ(96, entry2->size);
  CG_ASSERT_INT_EQ(1, entry2->in_use);
  CG_ASSERT_PTR_EQ(entry1, entry2->prev);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_coalesce_both(void)
{
  void *blk1, *blk2, *blk3, *blk4;
  struct metadata *entry1, *entry2;

  _reset_if_needed();

  blk1 = rhmalloc(80);
  blk2 = rhmalloc(88);
  blk3 = rhmalloc(96);
  blk4 = rhmalloc(104);

  /* free 1 and 3, then 2 so two blocks need to be coalesced in a single free.
   */
  rhfree(blk1);
  rhfree(blk3);
  rhfree(blk2);

  entry1 = freelist_head();
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);
  CG_ASSERT_INT_EQ(80 + 88 + 96 + 2 * META_SIZE, entry1->size);
  CG_ASSERT_INT_EQ(0, entry1->in_use);

  entry2 = freelist_head()->next;
  CG_ASSERT_INT_EQ(104, entry2->size);
  CG_ASSERT_INT_EQ(1, entry2->in_use);
  CG_ASSERT_PTR_EQ(entry1, entry2->prev);

  /* now delete block 4, which should return us to a completely free memory. */
  rhfree(blk4);
  CG_ASSERT_PTR_EQ(HEAP_MEM_START, entry1);
  CG_ASSERT_INT_EQ(MAX_HEAP_SIZE - META_SIZE, entry1->size);
  CG_ASSERT_INT_EQ(0, entry1->in_use);
  CG_ASSERT_PTR_EQ(0, entry1->next);
  CG_ASSERT_PTR_EQ(0, entry1->prev);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_too_small_blocks(void)
{
  void *blk1, *blk1_v2;
  struct metadata *entry1, *entry2, *split;

  _reset_if_needed();

  blk1 = rhmalloc(ALIGNMENT);
  rhmalloc(ALIGNMENT);

  rhfree(blk1);
  blk1_v2 = rhmalloc(ALIGNMENT + 1);
  /* This might seem obvious, but realize that it might seem like there is
   * enough space if you don't consider that the metadata needs to be stored as
   * well. */
  CG_ASSERT(blk1 != blk1_v2);

  _reset_if_needed();

  blk1 = rhmalloc(ALIGNMENT * 10);
  rhmalloc(ALIGNMENT);
  entry2 = freelist_head()->next;
  rhfree(blk1);
  blk1_v2 = rhmalloc(ALIGNMENT * 9);
  entry1  = freelist_head();

  CG_ASSERT(blk1 == blk1_v2);
  /* If we allow the block to be split into 2, we'll produce a remainder block
   * with a size too small to actually hold the metadata. This can produce
   * memory corruption, but even if it doesn't, it's silly to allow the
   * existence of an entry in our freelist that can't actually store data
   * because it doesn't really have any free space in it. The solution is simple
   * - if the "remainder" block is too small to be useful, we just allocate the
   * entire block for the request (not splitting) even if that's a little more
   * memory than the user requested. */
  CG_ASSERT_INT_EQ(ALIGNMENT * 10, entry1->size);
  CG_ASSERT_PTR_EQ(entry2, entry1->next);

  _reset_if_needed();

  blk1 = rhmalloc(ALIGNMENT * 10 + sizeof(struct metadata));
  rhmalloc(ALIGNMENT);
  entry2 = freelist_head()->next;
  rhfree(blk1);
  blk1_v2 = rhmalloc(ALIGNMENT * 10);
  entry1  = freelist_head();
  CG_ASSERT(blk1 == blk1_v2);

  /* This is the same as the previous case, only difference is that it is almost
   * big enough but not quite. It is big enough to store the metadata, but only
   * barely.  So the actualy data storage size would still be 0. As before, we
   * should just refuse to split the block. */
  CG_ASSERT_INT_EQ(ALIGNMENT * 10 + sizeof(struct metadata), entry1->size);
  CG_ASSERT_PTR_EQ(entry2, entry1->next);

  _reset_if_needed();

  blk1 = rhmalloc(ALIGNMENT * 10 + sizeof(struct metadata));
  rhmalloc(ALIGNMENT);
  entry2 = freelist_head()->next;
  rhfree(blk1);
  blk1_v2 = rhmalloc(ALIGNMENT * 9);
  entry1  = freelist_head();

  CG_ASSERT(blk1 == blk1_v2);

  /* now the block is just big enough that it is allowed to be split into 2
   * blocks.*/
  CG_ASSERT_INT_EQ(ALIGNMENT * 9, entry1->size);
  split = entry1->next;
  CG_ASSERT_INT_EQ(ALIGNMENT, split->size);
  CG_ASSERT_INT_EQ(0, split->in_use);
  CG_ASSERT_PTR_EQ(entry2, split->next);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_run_out_of_memory(void)
{
  int error;
  _reset_if_needed();

  CG_ASSERT_PTR_NNUL(rhmalloc(800));
  while(rhmalloc(800))
    ;

  CG_ASSERT_MSG(rhmalloc(800) == 0,
                "Check that mallocs fail after running out of memory");
  error = errno;
  CG_ASSERT_INT_EQ(ENOMEM, error);

  rhfree_all();
  return CG_TEST_PASSED;
}

static int
test_stress_big_heap_chunks(void)
{
  const int STEP         = 8;
  static int step_perm[] = {3, 4, 1, 2, 0, 5, 6};
  int size, i;
  void *ptr[STEP];

  _reset_if_needed();

  size = MAX_HEAP_SIZE / STEP;

  /* allocate size bytes (STEP-1) times */
  for(i = 0; i < STEP - 1; i++) {
    ptr[i] = rhmalloc(size);
    CG_ASSERT_MSG(ptr[i] != 0,
                  "Check if we can allocate all of heap in %d steps", STEP);
  }

  /* try to allocate size bytes for last step (it should fail due to rounding
   * and metadata)*/
  ptr[STEP - 1] = rhmalloc(size);
  CG_ASSERT_MSG(
      ptr[STEP - 1] == 0,
      "Last alloc of step size should fail due to metadata and rounding size.");

  /* free allocated memory by permutation */
  for(i = 0; i < STEP - 1; i++) {
    rhfree(ptr[step_perm[i]]);
  }

  /* finally try to allocate big chunk of memory to check functionality of free.
   */
  size   = MAX_HEAP_SIZE * 0.9;
  ptr[0] = rhmalloc(size);
  CG_ASSERT_MSG(
      ptr[0] != 0,
      "After freeing all of memory, we should be able to reallocate all.");

  return CG_TEST_PASSED;
}

static int
test_stress_no_overlap(void)
{
  const int LOOPCNT     = 100;
  const int BUFLEN      = 100;
  const int ALLOC_SIZE  = MAX_HEAP_SIZE / 2;
  const int ALLOC_CONST = 500;

  int size    = 0;
  int itr     = 0;
  int i       = 0;
  int j       = 0;
  int randvar = 0;
  int count   = 0;

  // store pointers to allocated memory segments
  void *ptr[BUFLEN];
  long int global[LOOPCNT][2];

  srand(time(0));
  _reset_if_needed();

  for(j = 0; j < LOOPCNT; j++) {
    global[j][0] = -1;
    global[j][1] = -1;
  }

  for(i = 0; i < BUFLEN; i++) {
    ptr[i] = 0;
  }

  for(i = 0; i < LOOPCNT; i++) {
    /* randomly choose an index for alloc/free */
    itr = rand() % BUFLEN;

    /* flip a coin to decide alloc/free */
    randvar = rand() % (2 * ALLOC_CONST);

    if(randvar < ALLOC_CONST && ptr[itr] == 0) {
      size     = (rand() % ALLOC_SIZE) + 1;
      ptr[itr] = rhmalloc(size);

      /* it is possible the allocation could fail because we've allocated the
       * max already. */
      if(ptr[itr] == 0)
        continue;

      /* If we did allocate, check every other existing allocation to ensure the
       * region we were given does not overlap with other regions. */
      count++;
      for(j = 0; j < i; j++) {
        if(global[j][0] == -1)
          continue;

        if((((long int)ptr[itr] >= global[j][0]) &&
            ((long int)ptr[itr] <= global[j][1])) ||
           (((long int)ptr[itr] + size >= global[j][0]) &&
            ((long int)ptr[itr] + size <= global[j][1]))) {
          CG_ASSERT_MSG(0, "Allocation regions overalp");
        }
        global[i][0] = (long int)ptr[itr];
        global[i][1] = (long int)ptr[itr] + size;
      }
    } else if(randvar >= ALLOC_CONST && ptr[itr] != 0) {
      /* free memory */
      for(j = 0; j < i; j++) {
        if(global[j][0] == (long int)ptr[itr]) {
          global[j][0] = -1;
          global[j][1] = -1;
        }
      }
      rhfree(ptr[itr]);
      ptr[itr] = 0;
    }
  }

  rhfree_all();
  return count > 0 ? CG_TEST_PASSED : CG_TEST_FAILED;
}

struct cg_test_suite *
rhmalloc_test_suite(void)
{
  struct cg_test_suite *ts = cg_test_suite_new("RH Malloc Test Suite", 0);

  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_basic_alloc", test_basic_alloc, 5,
      "Test that basic allocation splits the entire memory correctly.");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_two_allocs", test_two_allocs, 10,
      "Test the two allocations in a row split memory correctly.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_basic_free", test_basic_free, 5,
                              "Test a simple malloc followed by a free.");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_more_free", test_more_free, 10,
      "Test more cases of malloc/free combinations and block reuse.")
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_alignment", test_alignment, 10,
      "Test that the allocation of an aligned size will be aligned up.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_coalesce_right", test_coalesce_right,
                              10, "Test basic coalescing to the right.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_coalesce_left", test_coalesce_left, 10,
                              "Test basic coalescing to the left.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_coalesce_both", test_coalesce_both, 20,
                              "Test coalescing in both directions.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_too_small_blocks",
                              test_too_small_blocks, 20,
                              "Test trying to allocate too small blocks.");
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_run_out_of_memory", test_run_out_of_memory, 10,
      "Check that we run out of memory with error reporting.");
  CG_SUITE_CREATE_GRADED_TEST(ts, "test_stress_big_heap_chunks",
                              test_stress_big_heap_chunks, 20,
                              "Test if allocating all of memory then freeing "
                              "all restores original state.")
  CG_SUITE_CREATE_GRADED_TEST(
      ts, "test_stress_no_overlap", test_stress_no_overlap, 20,
      "Test that allocated regions does not overlap with each other.");
  return ts;
}

int
main(int argc, char **argv)
{
#ifdef BUILD_GRADESCOPE
  struct cg_test_suite *ts = rhmalloc_test_suite();
  int rc                   = cg_test_suite_runall(ts);

  cg_test_suite_summarize(ts);
  cg_test_suite_gradescopify_tests(ts, "rhmalloc_tests.run.json");

  cg_test_suite_remove(ts);
  return (rc > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
#else
  int rc                  = 0;
  struct cg_project *proj = cg_project_new("RHMalloc Implementation");
  cg_project_add_suite(proj, rhmalloc_test_suite());

  cg_project_runall(proj);
  cg_project_summarize(proj);

  rc = proj->num_failures > 0 ? 1 : 0;
  cg_project_del(proj);
  return rc;
#endif
}
