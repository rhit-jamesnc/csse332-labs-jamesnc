/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author Noah James
 * @date   3/18/26
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <bits/time.h>

#include "reading.h"

int
open_file(const char *name)
{
  return open(name, O_RDONLY);
}

ssize_t
get_file_size(int fd)
{
  struct stat sb;

  if(fstat(fd, &sb) == -1) {
    return -1;
  }

  if(!S_ISREG(sb.st_mode)) {
    return -1;
  }

  return sb.st_size;
}

ssize_t
read_bytes(int fd, char *buf, ssize_t len, size_t incr)
{
  // TODO: Complete this with your instructor
  ssize_t ret = 0, rlen = 0;

  while (len > 0 && (ret = read(fd, buf, incr))) {
    if (ret == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("Major error while reading file");
      return -1;
    }

    buf += ret;
    rlen += ret;
    len -= ret;
  }

  return rlen;
}

static double
_subtract_timspec(struct timespec t1, struct timespec t2)
{
  struct timespec diff;

  diff.tv_sec  = t1.tv_sec - t2.tv_sec;
  diff.tv_nsec = t1.tv_nsec - t2.tv_nsec;
  if(diff.tv_nsec < 0) {
    // we need to subtract a second out and then adjust the remainder
    diff.tv_nsec += 1000000000;
    diff.tv_sec--;
  }

  return (double)diff.tv_sec + (double)diff.tv_nsec / 1e09;
}

int
_main(int argc, char **argv)
{
  int fd;
  int rc = EXIT_SUCCESS;
  char *endptr;
  ssize_t fsize;
  ssize_t blk              = 1;
  struct timespec ts_start = {0, 0}, ts_end = {0, 0};
  char *buf;
  ssize_t tot = 0;
  ssize_t ret;

  // TODO: Please comment out this line when you implement the last step in
  // this file.
  // (void)_subtract_timspec(ts_start, ts_end);

  if(argc > 1) {
    errno = 0;
    blk   = strtoll(argv[1], &endptr, 10);
    if(errno || endptr < (argv[1] + strlen(argv[1]))) {
      fprintf(
          stderr,
          "[ERROR] Argument provided could not be parsed into an integer.\n");
      return EXIT_FAILURE;
    }
  }

  printf("[LOG] Using a chunk size of %ld\n", blk);
  fd = open_file("large.dat");
  if(fd == -1) {
    fprintf(stderr, "[ERROR]: Failed to open file.dat!\n");
    return EXIT_FAILURE;
  }

  if((fsize = get_file_size(fd)) == -1) {
    fprintf(stderr, "[ERROR]: Failed to get file size of file.dat!\n");
    close(fd);
    return EXIT_FAILURE;
  }

  // TODO:
  // =====
  //  Add code here to read all of the bytes of the input file fd.

  printf("[LOG] File is of size %ld bytes, allocating buffer.\n", fsize);

  buf = malloc(fsize);
  if(!buf) {
    fprintf(stderr, "[ERROR]: malloc failed.\n");
    // be a good person and do the following...
    close(fd);
    return EXIT_FAILURE;
  }

  // HINT:
  // =====
  // To measure time and print it, use the following:
  //
  // Add #include <time.h> if it's not there.
  //

  clock_gettime(CLOCK_MONOTONIC, &ts_start);

  while((ret = read_bytes(fd, buf, fsize, blk))) {
    tot += ret;

    if(ret == -1) {
      fprintf(stderr, "[ERROR]: read failed.\n");
      rc = EXIT_FAILURE;
    } else {
      printf("[LOG] Successfully read %ld bytes.\n", tot);
    }
  }

  //
  //   THING YOU'D LIKE TO MEASURE HERE
  //
  // TODO:
  // =====
  //    PLEASE USE THE SAME FPRINTF STATEMENT BELOW AS THE GRADING SCRIPT
  //    DEPENDS ON IT.
  //
  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  fprintf(stderr, "%lf seconds time elapsed\n",
           _subtract_timspec(ts_end, ts_start));
  
  //what goes here?
  free(buf);

  close(fd);
  return rc;
}
