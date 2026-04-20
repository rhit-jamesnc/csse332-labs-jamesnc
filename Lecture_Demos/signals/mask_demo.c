#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
mask_signal(int signum)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
    perror("sigprocmask:");
}

static void
unmask_signal(int signum)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, signum);
  if(sigprocmask(SIG_UNBLOCK, &mask, NULL) < 0)
    perror("sigprocmask:");
}

static void
setsighandler(int signum, void (*handler)(int))
{
  struct sigaction act;

  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  sigaction(signum, &act, NULL);
}

volatile sig_atomic_t num_sigint = 0;

void
handle_sig_int(int sig)
{
  // Again, bad idea to use printf here but we'll overlook that for
  // demonstration purposes.
  num_sigint++;
}

int
main(int argc, char **argv)
{
  int last_sigint = num_sigint;

  setsighandler(SIGINT, handle_sig_int);
  printf("Process %s (%d) started...\n", argv[0], getpid());

  while(num_sigint < 3) {
    if(num_sigint != last_sigint) {
      last_sigint = num_sigint;
      printf("Ouch, received interrupt signal number %d\n", num_sigint);
    }
  }
  last_sigint = num_sigint;
  printf("Ouch, received interrupt signal number %d\n", num_sigint);

  printf("[%s:%d] Tired of you interrupting me...\n", argv[0], getpid());
  mask_signal(SIGINT);
  printf("[%s:%d] Let's see you try now...\n", argv[0], getpid());
  sleep(5);
  unmask_signal(SIGINT);
  printf("[%s:%d] Back to receiving interrupts!\n", argv[0], getpid());
  while(num_sigint < 6) {
    if(num_sigint != last_sigint) {
      last_sigint = num_sigint;
      printf("Ouch, received interrupt signal number %d\n", num_sigint);
    }
  }
  last_sigint = num_sigint;
  printf("Ouch, received interrupt signal number %d\n", num_sigint);

  printf("[%s:%d] I am so tired of you right now!\n", argv[0], getpid());
  setsighandler(SIGINT, SIG_IGN);
  while(num_sigint < 9) {
    if(num_sigint != last_sigint) {
      last_sigint = num_sigint;
      printf("Ouch, received interrupt signal number %d\n", num_sigint);
    }
  }

  exit(0);
}
