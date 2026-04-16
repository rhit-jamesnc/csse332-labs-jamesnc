/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef RF_LOAD_H
#define RF_LOAD_H

#include <stddef.h>

// forward declarations.
struct rf_parse_state;
struct rf_hdr;

/**
 * A simple structure that holds the locations of the code and globals regions
 * of a given process while it is in execution.
 */
struct rf_exec_state {
  void *code;  //!< The starting address of the code region.
  size_t clen; //!< The length of the code region in bytes.
  void *data;  //!< The starting address of the data region, if any.
  size_t dlen; //!< The length of the data region in bytes.
  int rv;      //!< The return variable of the process after execution.
};

/**
 * Load a code-only virtual process and execute it.
 *
 * @param path  The path to the RF executable to load.
 * @param exst  A pointer to an execution state to fill in.
 *
 * @return 0 on success, RF_Error on failure.
 */
int rf_exec_code_only(const char *path, struct rf_exec_state *exst);

/**
 * Load a process from a given executable.
 *
 * @param path  The path to the RF executable to load.
 * @param exst  A pointer to an execution state to fill in.
 *
 * @return 0 on success, RF_Error on failure.
 */
int rf_exec(const char *path, struct rf_exec_state *exst);

/**
 * Unmap an execution state after if has been created.
 *
 * @param exst  A pointer to the execution state to Unmap
 *
 * @return 0 on success, -1 on failure and sets errno.
 */
int rf_unmap_state(struct rf_exec_state *exst);

/**
 * Caculate the number of pages needed to load a RF binary.
 *
 * @param ps  A parser state that corresponds to the binary we're loading.
 *
 * @return The number of pages needed to load the code and globals regions of
 *          the binary in question.
 */
int calc_npages(struct rf_parse_state *ps);

/**
 * Load the data region into memory and copy over its content.
 *
 * @param ps    The current parser state we are working off of.
 * @param hdr   The current RF header that has been parsed and validated.
 * @param code  The start of the code region for which this is being loaded.
 * @param len   A pointer to an integer that will hold the length of the
 *              region.
 *
 * @return a valid pointer to a data region on success and sets `len`, 0 on
 *  failure and sets `ps->error`.
 */
void *rf_load_data(struct rf_parse_state *ps, void *code, int *len);

/**
 * Load the code region into memory and copy over its content.
 *
 * @param ps    The current parser state we are working off of.
 * @param code  An optional hint to the desired start of this code region.
 * @param len   A pointer to an integer to hold the length of the code region.
 *
 * @return a valid pointer to a code region on success and sets `len`, 0 on
 *  failure and sets `ps->error`.
 */
void *rf_load_code(struct rf_parse_state *ps, void *code, int *len);

#endif // rh_load.h
