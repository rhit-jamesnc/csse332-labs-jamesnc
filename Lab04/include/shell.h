/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef _SHELL_H
#define _SHELL_H

/**
 * Get the line from the command prompt.
 *
 * @return a STATIC string that contains the line the user typed in the prompt.
 */
char *get_prompt_line(void);

/**
 * Process a given a command and possibly execute.
 *
 * @warning
 *    This function might change the content of the cmd string, so if you need
 *    it to persist, make sure to copy before calling here.
 *
 * @param cmd A string (usually obtained from get_prompt_line) that is the
 *            user's command.
 *
 * @return nothing.
 */
void process_command(char *cmd);

/**
 * Generate the arguments for the exec family of system calls.
 *
 * This function will parse the cmd command and extract all of its arguments
 * from it. It fills out the argv argument with a list of strings that are
 * ready to be used with the exec family of system calls.
 *
 * @note
 *   It is ok if this function modifies the command cmd. We do not need it
 *   beyond this function.
 *
 * @param cmd   The original command to parse.
 * @param argv  The array of strings fill out with arguments.
 *
 * @return The number of parsed arguments.
 */
int generate_exec_args(char *cmd, char *argv[]);

/**
 * Start a foreground command.
 *
 * @param cmd   The command to start passed from readline.
 *
 * @return the command's exit code if it existed, -1 otherwise.
 */
int start_fg_command(char *cmd);

/**
 * Start a background command.
 *
 * @param cmd   The command to start passed from readline.
 */
void start_bg_command(char *cmd);

#endif // shell.h
