/************************************************************************************
 *
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * Should you find any bugs in this file, please contact your instructor as
 * soon as possible.
 *
 ***********************************************************************************/

#include "proc_read.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static void *
__get_ptr_from_str(const char *str)
{
  unsigned long long addr;

  errno = 0;
  addr  = strtoull(str, 0, 16);
  if(errno) {
    perror("strtoull: ");
    exit(EXIT_FAILURE);
  }
  return (void *)addr;
}

int
open_pmaps_file(struct program_info *pinfo, pid_t pid)
{
  // TODO: REMOVE THIS BEFORE STARTING, THIS IS TO SILENCE COMPILER WARNINGS
  char path[256];
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    return -1;
  }
  fclose(fp);

  pinfo->pid = pid;
  pinfo->ready = 0;
  return 0;
}

int
parse_pmaps_file(struct program_info *pinfo)
{
  pinfo->ready = 1;
  return 0;
}

void *
get_code_start(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    int n = sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (n == 6 && name[0] == '/' && perms[2] == 'x') {
      char *dash = strchr(addr, '-');
      if (dash) { *dash = '\0'; res = __get_ptr_from_str(addr); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_code_end(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    int n = sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (n == 6 && name[0] == '/' && perms[2] == 'x') {
      char *dash = strchr(addr, '-');
      if (dash) { res = __get_ptr_from_str(dash + 1); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_globals_start(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    int n = sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (n == 6 && name[0] == '/' && perms[1] == 'w') {
      char *dash = strchr(addr, '-');
      if (dash) { *dash = '\0'; res = __get_ptr_from_str(addr); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_globals_end(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    int n = sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (n == 6 && name[0] == '/' && perms[1] == 'w') {
      char *dash = strchr(addr, '-');
      if (dash) { res = __get_ptr_from_str(dash + 1); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_stack_start(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (strstr(name, "[stack]")) {
      char *dash = strchr(addr, '-');
      if (dash) { *dash = '\0'; res = __get_ptr_from_str(addr); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_stack_end(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (strstr(name, "[stack]")) {
      char *dash = strchr(addr, '-');
      if (dash) { res = __get_ptr_from_str(dash + 1); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_heap_start(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (strstr(name, "[heap]")) {
      char *dash = strchr(addr, '-');
      if (dash) { *dash = '\0'; res = __get_ptr_from_str(addr); }
      break;
    }
  }
  fclose(fp);
  return res;
}

void *
get_heap_end(struct program_info *pinfo)
{
  pid_t pid = (pinfo == NULL) ? getpid() : pinfo->pid;
  char path[256], line[512], addr[128], perms[10], off[32], dev[32], ino[32], name[256];
  void *res = NULL;
  snprintf(path, sizeof(path), "/proc/%d/maps", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) return NULL;
  while (fgets(line, sizeof(line), fp)) {
    name[0] = '\0';
    sscanf(line, "%s %s %s %s %s %s", addr, perms, off, dev, ino, name);
    if (strstr(name, "[heap]")) {
      char *dash = strchr(addr, '-');
      if (dash) { res = __get_ptr_from_str(dash + 1); }
      break;
    }
  }
  fclose(fp);
  return res;
}
