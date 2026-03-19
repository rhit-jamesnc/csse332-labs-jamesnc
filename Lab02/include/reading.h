/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author Noah James
 * @date   3/18/26
 */
#ifndef READING_H
#define READING_H

#include <sys/types.h>

/**
 * Open a file with a given name.
 *
 * This function assumes the file already exists.
 *
 * @param name  A string representation the name of the file.
 *
 * @return a file descriptor number on success, -1 on failure.
 */
int open_file(const char *name);

/**
 * Find the size of a given file in bytes.
 *
 * @param fd  The file descriptor for the file.
 *
 * @return the number of bytes in the file on success, -1 on failure.
 */
ssize_t get_file_size(int fd);

/**
 * Read bytes from a file into a user provided buffer.
 *
 * This function should serve as a generic function to read bytes in certain
 * increments into a buffer.
 *
 * @warning
 *   This function performs no error checking on whether `buf` is big enough to
 *   read the totality of the bytes.
 *
 * @param fd    The file descriptor for the file to read from.
 * @param buf   The buf to read into, must be allocated by the user.
 * @param len   The number of bytes to read.
 * @param incr  The increment of bytes to read every iteration.
 *
 * @return the number of bytes read in total on success, -1 on error.
 */
ssize_t read_bytes(int fd, char *buf, ssize_t len, size_t incr);

/**
 * This is the function that the file runner will call. This can be effectively
 * treated as the main function of this file.
 *
 * @param argc  The argument count passed directly from caller.
 * @param argv  The argument vector passed directly from caller.
 *
 * @return 0 on success, error code on failure.
 */
int _main(int argc, char **argv);

#endif
