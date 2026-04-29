/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/22/26
 */
#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "shell.h"
#include "sig.h"

#define SHELL_PROMPT "rhsh"
// the maximum number of arguments, including the '&' for background commands
// and the null terminateor.
#define MAX_ARGS 16

char *
get_prompt_line(void)
{
  static char *line = 0;
  char *cmd         = 0;
  char *end         = 0;

  if(line) {
    free(line);
    line = 0;
  }

  line = readline(SHELL_PROMPT " $ ");

  // skip if just spaces or starts with newline
  cmd = line;
  while(*cmd && (isspace(*cmd) || *cmd == '\n'))
    cmd++;

  if(!*cmd)
    return 0;

  // remove trailing spaces
  end = cmd + strlen(cmd);
  while(end != cmd && isspace(*end)) {
    *end = 0;
    end--;
  }

  // can't reach here with everything being spaces because the first loop would
  // have caught that.

  // check if we'd want to exit
  if(!strncmp(cmd, "exit", 4) || !strncmp(cmd, "quit", 4)) {
    if(line)
      free(line);
    exit(EXIT_SUCCESS);
  }

  // Keep this here as a helper when testing with simpleshell, it allows you to
  // use the arrow keys to navigate your command history.
  add_history(cmd);
  return cmd;
}

void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0);
  errno = saved_errno;
}

void
process_command(char *cmd)
{
  setsighandler(SIGCHLD, handle_sigchld);

  char *end;
  printf("Received from the shell the command: %s\n", cmd);

  if(cmd[strlen(cmd) - 1] == '&') {
    cmd[strlen(cmd) - 1] = 0;
    // remove extra white spaces
    end                  = cmd + strlen(cmd) - 1;
    while(end != cmd && isspace(*end)) {
      *end = 0;
      end--;
    }
    start_bg_command(cmd);
  } else {
    start_fg_command(cmd);
  }
}

int
generate_exec_args(char *cmd, char *argv[])
{
  // TODO:
  // =====
  //  Implement this function...

  char *curr = cmd;
  argv[0] = cmd;

  int i = 1;
  while(*curr != '\0') {

    if(isspace(*curr)) {
      *curr = '\0';

      if(*(curr+1) != '\0') {
        argv[i++] = curr + 1;
      }
    }
    curr++;
  }
  argv[i] = NULL;
  return i;
}

int
start_fg_command(char *cmd)
{
  // TODO:
  // =====
  //   Implement code to start a foreground command.
  char *argv[MAX_ARGS];
  
  int count = 0;
  char *token = strtok(cmd, " ");
  while (token != NULL && count < MAX_ARGS - 1) {
    argv[count++] = token;
    token = strtok(NULL, " ");
  }
  argv[count] = NULL;

  if (count <= 0) {
    return -1;
  }

  printf("Running foreground command %s with %d arguments\n", argv[0], count);

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    execvp(argv[0], argv);
    
    perror("execvp");
    exit(EXIT_FAILURE);
  } else {
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      int exit_code = WEXITSTATUS(status);
      
      if (exit_code == 0) {
        printf("Command %s exited successfully\n", argv[0]);
      } else {
        printf("Command %s exited with code %d\n", argv[0], exit_code);
      }
      return exit_code;
    }

    return -1;
  }
}

void
start_bg_command(char *cmd)
{
  // TODO:
  // =====
  //   Implement code to start a background command.
  char *argv[MAX_ARGS];
  char cmd_copy[1024];

  memset(cmd_copy, 0, sizeof(cmd_copy));
  strncpy(cmd_copy, cmd, 1024);
  
  int len = strlen(cmd);
  cmd[len] = ' ';
  cmd[len+1] = '&';
  cmd[len+2] = '\0';

  int count = 0; 
  char *token = strtok(cmd, " ");
  while (token != NULL && count < MAX_ARGS - 1) {
      argv[count++] = token;
      token = strtok(NULL, " ");
  }
  argv[count] = NULL;

  if (count <= 0) {
    return;
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return;
  }

  if (pid == 0) {
    printf("Running background command %s with %d arguments\n", argv[0], count);
    fflush(stdout);

    for (int i = 0; i < count; i++) {
      if (argv[i] != NULL && strcmp(argv[i], "&") == 0) {
        argv[i] = NULL;
        break;
      }
    }

    pid_t worker_pid = fork();

    if (worker_pid == 0) {
      execvp(argv[0], argv);
      perror("execvp");
      exit(EXIT_FAILURE);
    } else {
      waitpid(worker_pid, NULL, 0);
      
      printf("Background command %s finished\n", cmd_copy);
      fflush(stdout);
      
      exit(EXIT_SUCCESS);
    }
  } else {
    return;
  }
}
