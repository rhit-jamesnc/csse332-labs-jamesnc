#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

// A context is ready to swap in
#define CTX_READY 1
// A context is ready to be cleaned up.
#define CTX_DONE 2

#define CTX_STACK_SZ 4096

#define CTX_P1 0
#define CTX_P2 1

// Save the currently running context
static int current_context = -1;
static ucontext_t *curr;
// The state of each context.
static int ctx_state[2];

// We want to create two contexts, so we create p1, p2, and reserve one for
// the main context that is running by default.
ucontext_t p1, p2, mn;

void
finish_task(void)
{
  printf(" Context %s is done, should let the main task know!\n",
         (current_context == CTX_P1) ? "P1" : "P2");
  ctx_state[current_context] = CTX_DONE;
  swapcontext(curr, &mn);
}

void
p1_func(const char *name)
{
  printf(" %s: Context task started!\n", name);
  for(int i = 0; i < 10; i++) {
    printf(" %s: Iteration %d: Swapping out back to the main context!\n", name,
           i);
    swapcontext(&p1, &mn);
    printf(" %s: Swapped back, will do some important stuff now!\n", name);
  }
  printf(" P1: I am done, so will make a final swap");
  finish_task();
}

void
p2_func(const char *name, int x)
{
  printf(" %s: Context task started!\n", name);
  for(int i = 0; i < x; i++) {
    printf(" %s: Iteration %d, Swapping out back to the main context!\n", name,
           i);
    swapcontext(&p2, &mn);
    printf(" %s: Swapped back, will do some important stuff now!\n", name);
  }
  printf(" %s: I am now all giddy, yay I can go home!\n", name);
  finish_task();
}

int
main(int argc, char **argv)
{
  int completed_tasks = 0;

  // Intialize things by inhering the current context.
  if(getcontext(&p1) < 0) {
    perror("getcontext");
    exit(EXIT_FAILURE);
  }
  if(getcontext(&p2) < 0) {
    perror("getcontext");
    exit(EXIT_FAILURE);
  }

  // create the two individual separate contexts
  p1.uc_stack.ss_sp   = malloc(CTX_STACK_SZ);
  p1.uc_stack.ss_size = CTX_STACK_SZ;
  makecontext(&p1, (void (*)(void))p1_func, 1, "P1");
  ctx_state[CTX_P1] = CTX_READY;

  p2.uc_stack.ss_sp   = malloc(CTX_STACK_SZ);
  p2.uc_stack.ss_size = CTX_STACK_SZ;
  makecontext(&p2, (void (*)(void))p2_func, 2, "P2", 3);
  ctx_state[CTX_P2] = CTX_READY;

  // Keep swapping contexts in until they are all done.
  current_context = -1;
  while(completed_tasks < 2) {
    // Find the next task to run
    // This will infinite loop if we land here with both tasks completed
    do {
      current_context = (current_context + 1) % 2;
    } while(ctx_state[current_context] != CTX_READY);

    if(current_context == CTX_P1)
      curr = &p1;
    else
      curr = &p2;
    // Save current context in mn and swap into p1, originally starting at
    // its own function
    printf("\t Swapping from main into context %s\n",
           (current_context == CTX_P1) ? "P1" : "P2");
    swapcontext(&mn, curr);

    // land back here after the context has swapped back.
    printf("\t Main resuming operation\n");

    // Check if current is done and mark it as completed.
    if(ctx_state[current_context] == CTX_DONE) {
      completed_tasks++;
    }
  }

  // both tasks are done, then clean them up
  free(p1.uc_stack.ss_sp);
  free(p2.uc_stack.ss_sp);
  return EXIT_SUCCESS;
}
