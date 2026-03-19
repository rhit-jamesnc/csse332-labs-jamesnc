/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author Noah James
 * @date   3/18/26
 */
#ifndef FREADING_H
#define FREADING_H

#include <stdio.h>
#include <sys/types.h>

/**
 * Open a file with a given name using standard I/O.
 *
 * This function assumes the file already exists.
 *
 * @param name  A string representation of the name of the file.
 *
 * @return A stream pointer on success, NULL on failure.
 */
FILE *open_stream(const char *name);

/**
 * Return the size of the stream.
 *
 * @param fp  A file pointer to the stream.
 *
 * @return the number of bytes in in the file on success, -1 on failure.
 */
ssize_t get_stream_size(FILE *fp);

/**
 * Read bytes from a file pointer stream into a user provided buffer.
 *
 * @warning
 *   This function performs no error checking on whether `buf` is big enough to
 *   read the totality of the bytes.
 *
 * @param fp    The file pointer to the open stream.
 * @param buf   The user buf to read into, must be allocated by the user.
 * @param len   The total number of bytes to read.
 * @param incr  The chunk size or increment number of bytes to read.
 *
 * @return the total number of bytes read on success, -1 on failure.
 */
ssize_t stream_read_bytes(FILE *fp, char *buf, ssize_t len, size_t incr);

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
