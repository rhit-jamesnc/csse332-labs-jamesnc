#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

void
basic_example(void)
{
  void *p = 0;
  int rc  = 0;
  int *array;

  p = mmap(0, getpagesize(), PROT_WRITE | PROT_READ,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(p == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // make an array of integers, how many integer could we have?
  array = (int *)p;
  for(int i = 0; i < getpagesize() / sizeof(int); i++) {
    array[i] = i;
  }

  // examing the arrary in gdb to verify it is here.
  printf("Array[5] = %d\n", array[5]);

  // done with memory, return the page
  rc = munmap(p, getpagesize());
  if(rc < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
}

void
mprotect_example(void)
{
  void *p    = 0;
  int *array = 0;
  int rc     = 0;

  p = mmap(0, getpagesize(), PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(p == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // make an array of integers, how many integer could we have?
  array = (int *)p;
  for(int i = 0; i < getpagesize() / sizeof(int); i++) {
    array[i] = i;
  }

  // What happens if we try to do this now?
  //  READ THE man page for mmap for this one, the behavior is not what you'd
  //  expect.
  // printf("Array[5] = %d\n", array[5]);

  // now area of memory should no longer be writable, but read-only
  rc = mprotect(p, getpagesize(), PROT_READ);
  if(rc < 0) {
    perror("mprotect");
    goto unmap_mem;
  }
  printf("Array[5] = %d\n", array[5]);

  // What happens if we try to change array now?
  // array[4] = -1;

unmap_mem:
  rc = munmap(p, getpagesize());
  if(rc < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
}

void
advanced_example(void)
{
  void *p            = 0;
  void *p_plus_three = 0;
  int rc             = 0;
  int fd             = 0;

  p = mmap(0, 4 * getpagesize(), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(p == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  printf("p is at %p\n", p);

  fd = open("csse332.txt", O_RDONLY);
  if(fd < 0) {
    perror("open");
    goto unmap_all;
  }

  p_plus_three = p + 3 * getpagesize();
  // What do you think will happen here? Note the new flag we're using
  // MAP_FIXED and the first argument to mmap, the address hint.
  p_plus_three = mmap(p_plus_three, getpagesize(), PROT_READ,
                      MAP_FIXED | MAP_PRIVATE, fd, 0);
  if(p_plus_three == MAP_FAILED) {
    perror("mmap");
    goto unmap_all;
  }

  // Examing the content of p_plus_three
  printf("Reading the page at %p as a string gives: %s\n", p_plus_three,
         (char *)p_plus_three);

  // unmap only p_plus_three
  rc = munmap(p_plus_three, getpagesize());
  if(rc < 0) {
    perror("munmap");
    // things are realy bad here, so exit directly
    exit(EXIT_FAILURE);
  }
  close(fd);

  // unmap the rest of memory, note the change in the length of the memory
  // region
  rc = munmap(p, 3 * getpagesize());
  if(rc < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
  return;

unmap_all:
  rc = munmap(p, 5 * getpagesize());
  if(rc < 0) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
}

int
main(int argc, char **argv)
{
  // basic_example();
  mprotect_example();
  // advanced_example();
  return EXIT_SUCCESS;
}
