/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   4/22/26
 */
#include <readline/history.h>

#include <shell.h>

int
main()
{
  char *line = 0;
  using_history(); // initialize the history state.
  while(1) {
    line = get_prompt_line();
    if(line)
      process_command(line);
  }
}
