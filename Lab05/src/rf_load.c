/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author Noah James
 * @date   5/13/26
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "rf_load.h"
#include "rf_parse.h"

int
calc_npages(struct rf_parse_state *ps)
{
  // TODO:
  // =====
  //  Implement this function to compute how many pages we'd need to store the
  //  code and globals regions of the CSSE332RF binary in question.
  // return 0;

  int np =0;

  struct rf_shdr shdr;

  //code
  if(rf_find_section_by_name(ps, &shdr, ".text") == 0) {
    np += (shdr.len + getpagesize() - 1) / getpagesize();
  }

  ps->error = RF_ERR_NULL;

  //any globals?
  if(rf_find_section_by_name(ps, &shdr, ".data") == 0) {
    np += (shdr.len + getpagesize() - 1) / getpagesize();
  }

  //rest error state to move forward
  ps->error = RF_ERR_NULL;

  return np;
}

void *
rf_load_code(struct rf_parse_state *ps, void *code, int *len)
{
  struct rf_shdr shdr;
  size_t pagesize = getpagesize();
  
  if (rf_find_section_by_name(ps, &shdr, ".text") != 0) {
    return NULL;
  }

  size_t total_len = ((shdr.len + pagesize - 1) / pagesize) * pagesize;
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  if (code != NULL) {
    flags |= MAP_FIXED;
  }

  void *mapped_addr = mmap(code, total_len, PROT_READ | PROT_WRITE, flags, -1, 0);
  if (mapped_addr == MAP_FAILED) {
    ps->error = RF_ERR_FILEIO;
    return NULL;
  }

  unsigned char *temp_buf = NULL;
  if (rf_read_section_body(ps, &shdr, &temp_buf) != 0) {
    munmap(mapped_addr, total_len);
    return NULL;
  }

  memcpy(mapped_addr, temp_buf, shdr.len);
  free(temp_buf);

  if (mprotect(mapped_addr, total_len, PROT_READ | PROT_EXEC) != 0) {
    perror("mprotect");
    munmap(mapped_addr, total_len);
    return NULL;
  }

  *len = shdr.len; 
  ps->error = RF_ERR_NULL;
  return mapped_addr;
}

typedef int (*start_func_t)();

int
rf_exec_code_only(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int rc;

  if ((rc = rf_parse_state_init(&ps, path)) != 0) return rc;
  if ((rc = rf_read_hdr(&ps, &hdr)) != 0) return rc;
  if ((rc = rf_validate_hdr(&hdr, &ps)) != 0) return rc;

  int clen_temp;
  exst->code = rf_load_code(&ps, NULL, &clen_temp);
  if (exst->code == NULL) {
    if (ps.fd >= 0) {
      close(ps.fd);
      ps.fd = -1;
    }
    return ps.error;
  }
  exst->clen = (size_t)clen_temp;

  exst->data = NULL;
  exst->dlen = 0;

  void *entry_address = (unsigned char *)exst->code + hdr.entry_offset;
  start_func_t start_fn = (start_func_t)entry_address;

  exst->rv = start_fn();

  if (ps.fd >= 0) {
    close(ps.fd);
    ps.fd = -1;
  }
  return 0;
}

void *
rf_load_data(struct rf_parse_state *ps, void *code, int *len)
{
  // TODO:
  // =====
  //  Implement code that would load the globals segment (if any) from the file
  //  into the page that starts right after the code region. Make sure to stay
  //  page aligned.
  struct rf_shdr shdr;

  if (rf_find_section_by_name(ps, &shdr, ".data") != 0) {
    *len = 0;
    return NULL;
  }

  void *target_addr = (unsigned char *)code + shdr.addr;

  void *data_map = mmap(target_addr, shdr.len, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

  if (data_map == MAP_FAILED) {
    ps->error = RF_ERR_LASTONE; 
    return NULL;
  }

  unsigned char *dest = (unsigned char *)data_map;
  if (rf_read_section_body(ps, &shdr, &dest) != 0) {
    munmap(data_map, shdr.len);
    return NULL;
  }

  *len = shdr.len;
  return data_map;
}

int
rf_exec(const char *path, struct rf_exec_state *exst)
{
  // TODO:
  // =====
  //  Implement code that loads and executes a process from a given binary
  //  path.
  struct rf_parse_state ps = {0};
  struct rf_hdr hdr        = {0};
  int err                  = 0;
  int temp_clen = 0;
  int temp_dlen = 0;

  // Load the parser state
  if(rf_parse_state_init(&ps, path)) {
    return ps.error;
  }

  // Read and validate the header
  if(rf_read_hdr(&ps, &hdr) || rf_validate_hdr(&hdr, &ps)) {
    err = ps.error;
    if (ps.fd >= 0) {
      close(ps.fd);
      ps.fd = -1;
    }
    return err;
  }

  // 1. Adjust memory so that we respect the locations of the code and globals
  //    required by the CSSE332 RF format.
  size_t pagesize = getpagesize();
  int total_pages = calc_npages(&ps);
  size_t total_mem_size = total_pages * pagesize;

  void *base_ptr = mmap(NULL, total_mem_size, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (base_ptr == MAP_FAILED) {
    if (ps.fd >= 0) {
      close(ps.fd);
      ps.fd = -1;
    }
    return RF_ERR_FILEIO;
  }

  // 2. Load the code segment from the binary.
  exst->code = rf_load_code(&ps, base_ptr, &temp_clen);
  exst->clen = (size_t)temp_clen;

  // 3. Load the globals segment (if any) from the binary into memory at a
  //    specific location.  
  exst->data = rf_load_data(&ps, base_ptr, &temp_dlen);
  exst->dlen = (size_t)temp_dlen;

  // 4. Find the entry point for the newly created virtual process.
  int (*entry_func)() = (int (*)()) ((char *)exst->code + hdr.entry_offset);

  // 5. Execute the process by simply calling the entry function and capturing
  //    the return value. Save the return value into `exst->rv`.
  exst->rv = entry_func();

  // DO NOT unmap the execution state, we do that in rf_unmap_state
  if (ps.fd >= 0) {
    close(ps.fd);
    ps.fd = -1;
  }
  return 0;
}

int
rf_unmap_state(struct rf_exec_state *exst)
{
  size_t pagesize = getpagesize();
  int err = 0;

  if(exst->code) {
    size_t code_page_len = ((exst->clen + pagesize - 1) / pagesize) * pagesize;
    if(munmap(exst->code, code_page_len)) {
      err = -1;
    }
  }

  if(exst->data) {
    size_t data_page_len = ((exst->dlen + pagesize - 1) / pagesize) * pagesize;
    if(munmap(exst->data, data_page_len)) {
      err = -1;
    }
  }

  return err;
}
