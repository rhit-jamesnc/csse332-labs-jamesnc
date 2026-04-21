/**
 * Copyright (c) 2026 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/20/26
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "vm.h"

/** The file descriptor for the swap space */
static int swap_fd = -1;

/** The page table */
static struct pte pgtbl[NUM_VIRT_PAGES];

/** The starting address of the memory area */
static uint64_t mem_start = 0;

/********************************************************************************
 * Helpers
 *******************************************************************************/
static char *
_int_to_str(int x)
{
  static char buff[128];
  char *p = buff + 127;
  int digit;

  *p = 0;
  if(!x) {
    *(--p) = '0';
    return p;
  }

  while(x) {
    digit = x % 10;
    p     = p - 1;
    *p    = '0' + digit;
    x     = x / 10;
  }
  return p;
}

static void
restore_sigact(int signum)
{
  struct sigaction act;

  memset(&act, 0, sizeof act);
  sigemptyset(&act.sa_mask);
  act.sa_handler = SIG_DFL;
  act.sa_flags   = 0;

  if(sigaction(signum, &act, NULL) == -1) {
    // skip cleanup since we're already messed up.
    _exit(EXIT_FAILURE);
  }
}

static int
open_swap(void)
{
  const char *swap = "dev.swap";
  int err, fd;

  fd = open(swap, O_RDWR | O_CREAT, S_IRWXU);
  if(fd == -1) {
    // propagate errno and just return -1;
    return -1;
  }

  if(ftruncate(fd, NUM_VIRT_PAGES * getpagesize()) == -1) {
    // proagate errno
    err = errno;
    close(fd);
    errno = err;
    return -1;
  }

  return fd;
}

int
init_vm(void)
{
  void *addr = 0;
  struct sigaction act;

  if(mem_start) {
    // already initialized
    errno = EINVAL;
    return -1;
  }

  if((swap_fd = open_swap()) == -1) {
    return -1;
  }

  addr = mmap(0, NUM_VIRT_PAGES * getpagesize(), PROT_NONE, MAP_SHARED, swap_fd,
              0);
  if(addr == MAP_FAILED) {
    // propagate errno, do not touch it.
    return -1;
  }
  mem_start = (uint64_t)addr;

  // TODO:
  // ======
  //  Initialize the page table
  for (int i = 0; i < NUM_VIRT_PAGES; i++) {
      pgtbl[i].state = UNUSED;
      pgtbl[i].freshness = 0;
      pgtbl[i].offset = i * getpagesize();
  }

  // set the segfault handler
  sigemptyset(&act.sa_mask);
  act.sa_flags     = SA_RESTART | SA_SIGINFO;
  act.sa_sigaction = segv_handler;
  if(sigaction(SIGSEGV, &act, NULL)) {
    perror("handler intialization failed:");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int
teardown_vm(void)
{
  if(!mem_start) {
    errno = EINVAL;
    return -1;
  }

  munmap((void *)mem_start, NUM_VIRT_PAGES * getpagesize());
  close(swap_fd);

  // write weird stuff in the page table
  memset(pgtbl, 5, sizeof(pgtbl));

  // reinitialize stuff
  swap_fd   = -1;
  mem_start = 0;
  return 0;
}

void
segv_handler(int sig, siginfo_t *si, void *unused)
{
  void *addr = 0;

  // Grab faulty address and calculate the page number
  addr = si->si_addr;
  // TODO:
  // =====
  //  Calculate the page number and check that is a valid one.
  //  Use `goto fail_on_segv` to exit with an error.
  uint64_t fault_addr = (uint64_t)addr;
  if(fault_addr < mem_start || fault_addr >= mem_start + (NUM_VIRT_PAGES * getpagesize())) {
    goto fail_on_segv;
  }
  int pagenum = (fault_addr - mem_start) / getpagesize();

  write(1, "In handler for a segmentation fault!\n",
        strlen("In handler for a segmentation fault!\n"));

  // TODO:
  // =====
  //  Uncomment this line after your start otherwise you will alaways fail.
  //  Note that if you don't map page correctly, you might end up in an
  //  infinite loop.
  if(pgtbl[pagenum].state == ACTIVE) {
    goto fail_on_segv;
  }

  // TODO:
  // =====
  //  Update the page table for that page.
  //  Make sure to do error checking.
  //
  // check for an edge case if we get here because of non page issue
  int active_count = 0;
  for(int i = 0; i < NUM_VIRT_PAGES; i++) {
    if (pgtbl[i].state == ACTIVE) {
      active_count++;
    }
  }

  for(int i = 0; i < NUM_VIRT_PAGES; i++) {
    if (pgtbl[i].state == ACTIVE && i != pagenum && pgtbl[i].freshness > 0) {
      pgtbl[i].freshness--;
    }
  }

  

  // TODO:
  // =====
  //   Decide if any pages need to be evicted.
  if(active_count >= NUM_PHYS_FRAMES) {
    int victim = -1;
    for(int i = 0; i < NUM_VIRT_PAGES; i++) {
      if (pgtbl[i].state == ACTIVE && pgtbl[i].freshness == 0) {
        victim = i;
        break;
      }
    }
    if(victim != -1) {
      evict_page(victim, &pgtbl[victim]);
    }
  }

  // TODO:
  // =====
  //  Map the page into memory using the `map_page` function.
  if(map_page(pagenum, &pgtbl[pagenum]) == -1) {
      goto fail_on_segv;
  }
  
  pgtbl[pagenum].state = ACTIVE;
  pgtbl[pagenum].freshness = NUM_PHYS_FRAMES;

  return;

fail_on_segv:
  restore_sigact(sig);
  raise(sig);
}

int
evict_page(int pagenum, struct pte *pte)
{
  // TODO:
  // =====
  //
  //  Call `munmap` to unmap the requested page and adjust the pte accordingly.
  void *pgaddr = (void *)mem_start + (pagenum * getpagesize());
  if (mmap(pgaddr, getpagesize(), PROT_NONE, MAP_SHARED | MAP_FIXED, swap_fd, pte->offset) == MAP_FAILED) {
      return -1;
  }

  pte->state = EVICTED;
  pte->freshness = 0;
  return 0;
}

int
map_page(int pagenum, struct pte *pte)
{
  void *pgaddr = 0;
  void *addr   = 0;

  pgaddr = (void *)mem_start + pagenum * getpagesize();
  addr   = mmap(pgaddr, getpagesize(), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_FIXED, swap_fd, pte->offset);
  if(addr == MAP_FAILED) {
    return -1;
  }

  return 0;
}

void *
give_me_pages(int num_pages)
{
  if(num_pages > NUM_VIRT_PAGES) {
    errno = ENOMEM;
    return 0;
  }

  // TODO:
  // =====
  //  Fix this line.
  //
  //  Do practically nothing, approach this lazily as memory is required, just
  //  promise the pages at some point.
  return (void *)mem_start;
}

/********************************************************************************
 * Grading Helpers
 *******************************************************************************/
struct pte *
get_pgtbl(void)
{
  (void)_int_to_str(0);
  return pgtbl;
}

int
get_swapfd(void)
{
  return swap_fd;
}

static const char *state_str[STATE_LAST_ONE] = {
    [UNUSED]  = "unused",
    [EVICTED] = "evicted",
    [ACTIVE]  = "active",
};

const char *
pg_state_str(unsigned char state)
{
  if(state < STATE_LAST_ONE)
    return state_str[state];
  return "ERROR";
}

uint64_t
get_memstart(void)
{
  return mem_start;
}
