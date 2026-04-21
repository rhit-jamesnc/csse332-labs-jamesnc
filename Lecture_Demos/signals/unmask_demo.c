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

int num_sigint = 0;

void
handle_sig_int(int sig)
{
  printf("Received interrupt signal, leaving on my volition...\n");
  _exit(0);
}

int
main(int argc, char **argv)
{
  char *name = argv[0] + 2; // skip the ./
  setsighandler(SIGINT, handle_sig_int);
  printf("Process %s (%d) started...\n", name, getpid());

  printf("[%s:%d] Will mask SIGINT while I do something important...\n", name,
         getpid());
  mask_signal(SIGINT);
  sleep(10);

  printf("[%s:%d] Done with important stuff, unmasking now...\n", name,
         getpid());
  unmask_signal(SIGINT);

  printf("[%s:%d] Okay, I have something really important to say now:\n", name,
         getpid());
  exit(0);
}
