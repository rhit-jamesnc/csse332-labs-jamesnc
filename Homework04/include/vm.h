/**
 * Copyright (c) 2026 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/20/26
 */

#ifndef _VM_H
#define _VM_H

#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#define NUM_VIRT_PAGES 15 //!< The number of virtual pages available
#define NUM_PHYS_FRAMES 3 //!< The maximum number of physical frames.

/**
 * Page state enum for describing the page state in memory.
 */
enum PAGE_STATE {
  UNUSED = 0, //!< Unused page so far, haven't been actively allocated.
  EVICTED,    //!< Page has been evicted to disk
  ACTIVE,     //!< Page is active

  STATE_LAST_ONE //!< Keep this at the end
};

/**
 * A page table entry.
 */
struct pte {
  unsigned char state;     //!< The state of the page.
  unsigned char freshness; //!< The "freshness" value of the page.
  off_t offset;            //!< The offset of the page in the swap space.
};

/**
 * Initialize the virtual memory subsystem.
 *
 * @return 0 on success, -1 on failure and sets errno.
 */
int init_vm(void);

/**
 *  Teardown the vm subsystem
 *
 *  @return 0 on success, -1 on failure and sets errno.
 */
int teardown_vm(void);

/**
 * The segmentation fault handler for lazy allocation and eviction.
 *
 * @param sig   The signal number for the offending signal.
 * @param si    More information about the offending signal, including bad
 *              address.
 * @param unused  Unussed context (typically ignored by handlers).
 *
 */
void segv_handler(int sig, siginfo_t *si, void *unused);

/**
 * Evict a page from physical memory to disk.
 *
 * @param pagenum The page number.
 * @param pte     The page's page table entry to update.
 *
 * @return 0 on success, -1 on failure and sets errno.
 */
int evict_page(int pagenum, struct pte *pte);

/**
 * Map a page either for the first time or from disk.
 *
 * @param pagenum   The page number.
 * @param pte       The page's page table entry to update.
 *
 * @return 0 on success, -1 on failure.
 */
int map_page(int pagenum, struct pte *pte);

/**
 * Give me some pages I request
 *
 * @param num_pages   The number of pages I am asking for.
 *
 * @return 0 on failure, a valid address to the start of the range of pages on
 *         success.
 */
void *give_me_pages(int num_pages);

/********************************************************************************
 * Grading Helpers
 *******************************************************************************/
// Grab a reference to the page table
struct pte *get_pgtbl(void);

// Grab a reference to the swap disk fd
int get_swapfd(void);

// String representation of a page's state.
const char *pg_state_str(unsigned char state);

// Get the starting address of the memory area
uint64_t get_memstart(void);

#endif /* vm.h */
