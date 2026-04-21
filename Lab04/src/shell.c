/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <ctype.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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

void
process_command(char *cmd)
{
  printf("Received from the shell the command: %s\n", cmd);

  if(cmd[strlen(cmd) - 1] == '&') {
    cmd[strlen(cmd) - 1] = 0;
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
  return 0;
}

int
start_fg_command(char *cmd)
{
  // TODO:
  // =====
  //   Implement code to start a foreground command.
  return -1;
}

void
start_bg_command(char *cmd)
{
  // TODO:
  // =====
  //   Implement code to start a background command.
}
