/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Implementation of the memory area with several types.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "rf.h"
#include "rf_parse.h"

static const char *rf_err_tostr[RF_ERR_LASTONE + 1] = {
    [RF_ERR_NULL]       = "No error",
    [RF_ERR_FNOFOUND]   = "File not found",
    [RF_ERR_INVALIDHDR] = "Invalid RF file header",
    [RF_ERR_FILEIO]     = "File access error on read",
    [RF_ERR_EOF]   = "End of file reached while performing a read operation",
    [RF_ERR_NOCHK] = "File header has not been checked yet",
    [RF_ERRR_FILESHORT] = "File too short to contain requested data",
    [RF_ERR_CLOSED]     = "Parser state is destroyed and can be reused",
    [RF_ERR_SEC_NFOUND] =
        "The section you're looking for is not found in the file",
    [RF_ERR_MALFORMED] = "Malformed RF file.",
    [RF_ERR_LASTONE]   = "Unknown error, check errno!",
};

static int
_set_file_offset(int fd, size_t offset)
{
  return lseek(fd, offset, SEEK_SET);
}

static int
_advance_file_offset(int fd, size_t offset)
{
  return lseek(fd, offset, SEEK_CUR);
}

static int
_reset_file_offset(int fd)
{
  return _set_file_offset(fd, 0);
}

static ssize_t
_get_file_size(int fd)
{
  struct stat stt;

  if(fstat(fd, &stt)) {
    return -1;
  }

  return stt.st_size;
}

static int
_rf_read(int fd, void *dst, size_t len)
{
  ssize_t rc = 0;
  while((rc = read(fd, dst, len)) < 0) {
    if(errno == EINTR) {
      continue;
    }
    return RF_ERR_FILEIO;
  }
  if(rc < len) {
    return RF_ERR_EOF;
  }
  return 0;
}

int
rf_advance_offset(struct rf_parse_state *st, unsigned int n)
{
  return _advance_file_offset(st->fd, n);
}

int
rf_parse_state_init(struct rf_parse_state *st, const char *name)
{
  st->error = RF_ERR_NULL;
  st->fd    = open(name, O_RDONLY);
  if(st->fd < 0) {
    st->error = RF_ERR_FNOFOUND;
    return -1;
  }
  strncpy(st->fname, name, 255);
  st->off = 0;
  return 0;
}

void
rf_parse_state_destroy(struct rf_parse_state *st)
{
  st->error = RF_ERR_CLOSED;
  close(st->fd);
  memset(st->fname, 0, 256);
  st->off = 0;
}

int
rf_read_hdr(struct rf_parse_state *st, struct rf_hdr *hdr)
{
  int rc;
  if(st->error != RF_ERR_NULL)
    return -1;

  // Check if we should reset the file offset back to 0.
  if(st->off) {
    if(_reset_file_offset(st->fd) == -1) {
      st->error = RF_ERR_FILEIO;
      return -1;
    }
    st->off = 0;
  }

  // Call the reading wrapper
  if((rc = _rf_read(st->fd, (void *)hdr, sizeof(struct rf_hdr)))) {
    st->error = rc;
    return rc;
  }
  st->off += sizeof *hdr;

  // copy the header over
  memcpy(&st->hdr, hdr, sizeof *hdr);
  return 0;
}

int
rf_validate_hdr(struct rf_hdr *hdr, struct rf_parse_state *st)
{
  if(memcmp(hdr->sig, RF_HDR_NAME, 7) || hdr->sig[7] != RF_HDR_VERSION ||
     hdr->sig[15] != 0 || memcmp(&hdr->sig[8], RF_HDR_MAGIC, 8)) {
    if(st)
      st->error = RF_ERR_INVALIDHDR;
    return -1;
  }

  return 0;
}

int
rf_read_section(struct rf_parse_state *st, struct rf_shdr *shdr,
                unsigned char **buf)
{
  int rc;
  if(st->error != RF_ERR_NULL)
    return -1;
  if(st->off == 0) {
    st->error = RF_ERR_NOCHK;
    return -1;
  }

  // First, grab the head of the section to know how long it would be.
  if((rc = _rf_read(st->fd, (void *)shdr, sizeof(struct rf_shdr)))) {
    st->error = rc;
    return rc;
  }
  st->off += sizeof *shdr;

  // Then, read the bytes of the section into the provided buffer after
  // allocating it.
  *buf = malloc(shdr->len);
  if(!*buf) {
    // This is a devastating error for us, so exit, no point in returning.
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // Reading loop, here we must check that the file contains the data.
  if((rc = _rf_read(st->fd, (void *)*buf, shdr->len))) {
    st->error = rc;
    return rc;
  }
  st->off += shdr->len;

  return 0;
}

int
rf_read_section_header(struct rf_parse_state *st, struct rf_shdr *shdr)
{
  int rc;
  if(st->error != RF_ERR_NULL)
    return -1;
  if(st->off == 0) {
    st->error = RF_ERR_NOCHK;
    return -1;
  }

  // First, grab the head of the section to know how long it would be.
  if((rc = _rf_read(st->fd, (void *)shdr, sizeof(struct rf_shdr)))) {
    st->error = rc;
    return rc;
  }
  st->off += sizeof *shdr;

  return 0;
}

int
rf_find_section_by_name(struct rf_parse_state *st, struct rf_shdr *shdr,
                        const char *name)
{
  int rc             = 0;
  struct rf_shdr tmp = {0};
  if(st->error != RF_ERR_NULL && st->error != RF_ERR_SEC_NFOUND)
    return -1;

  // First, reset the seeking to the start of the sections.
  if(_set_file_offset(st->fd, sizeof(struct rf_hdr)) == -1) {
    st->error = RF_ERR_FILEIO;
    rc        = -1;
    goto done;
  }
  st->off = sizeof(struct rf_hdr);

  // Now iterate over the sections until we find the right one.
  while(st->off < _get_file_size(st->fd)) {
    if(rf_read_section_header(st, &tmp)) {
      // rf_read_section_header would have set st->error
      return -1;
    }
    if(!strncmp((const char *)tmp.name, name, 16)) {
      // Found it
      *shdr = tmp;
      rc    = 0;
      goto done;
    }
    // move the offset forwards by the size of the header and its body.
    st->off += sizeof(struct rf_shdr) + tmp.len;
    // must also move the offset over the section's body.
    if(_advance_file_offset(st->fd, tmp.len) == -1) {
      st->error = RF_ERR_FILEIO;
      rc        = -1;
      goto done;
    }
  }

  // not found
  st->error = RF_ERR_SEC_NFOUND;
  rc        = -1;

done:
  if(_set_file_offset(st->fd, 0) == -1) {
    st->error = RF_ERR_FILEIO;
    rc        = -1;
  }
  st->off = 0;
  return rc;
}

int
rf_read_section_body(struct rf_parse_state *st, struct rf_shdr *shdr,
                     unsigned char **buf)
{
  int rc;
  if(st->error != RF_ERR_NULL)
    return -1;

  // Then, read the bytes of the section into the provided buffer after
  // allocating it.
  *buf = malloc(shdr->len);
  if(!*buf) {
    // This is a devastating error for us, so exit, no point in returning.
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // Set the file access to the start of the body.
  if(_set_file_offset(st->fd, shdr->offset) == -1) {
    st->error = RF_ERR_FILEIO;
    free(buf);
    return -1;
  }

  // Reading loop, here we must check that the file contains the data.
  if((rc = _rf_read(st->fd, (void *)*buf, shdr->len))) {
    st->error = rc;
    return rc;
  }
  st->off = shdr->offset + shdr->len;

  return 0;
}

const char *
rf_strerror(enum RF_ERROR err)
{
  return rf_err_tostr[err];
}
