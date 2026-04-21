/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef RF_PARSE_H
#define RF_PARSE_H

#include "rf.h"

enum RF_ERROR {
  RF_ERR_NULL = 0,   //!< No errors encountered.
  RF_ERR_FNOFOUND,   //!< File not found err.
  RF_ERR_INVALIDHDR, //!< The header in the file was invalid.
  RF_ERR_FILEIO,     //!< I/O error when accessing the file.
  RF_ERR_EOF,        //!< We have reached the end of the file.
  RF_ERR_NOCHK,      //!< Header has not been validated yet.
  RF_ERRR_FILESHORT, //!< The file does not contain enough data.
  RF_ERR_CLOSED,     //!< The parse state has been closed.
  RF_ERR_SEC_NFOUND, //!< The section asked to find is not in the file.
  RF_ERR_MALFORMED,  //!< Malformed RF file.

  RF_ERR_LASTONE, //!< I don't know what the error is, check errno.
};

/**
 * A structure to keep track of the parser's state.
 */
struct rf_parse_state {
  int fd;            //!< The file descriptor for the current parse.
  int off;           //!< The current offset in the file.
  int error;         //!< Any error encountered during the parse operation.
  struct rf_hdr hdr; //!< Keep a local copy of the header.
  char fname[256];   //!< The name of the file being parsed.
};

/**
 * Initialize a parser state with default values.
 *
 * @param st    The parser state to initialize.
 * @param name  The path to the file to parse.
 *
 * @return 0 on success, -1 on error and sets st->err.
 */
int rf_parse_state_init(struct rf_parse_state *st, const char *name);

/**
 * Destroy a parser state to be reused later on.
 *
 * @param st  The parser state to destroy.
 *
 * @return nothing.
 */
void rf_parse_state_destroy(struct rf_parse_state *st);

/**
 * Advance the offset into the parsed file by n bytes.
 *
 * @warning
 *  This will move starting from the current offset in the file.
 *
 * @param st  The current parser state  in use.
 * @param n   The number of bytes to advance forward.
 *
 * @return 0 on success, -1 on failure and set st->error.
 */
int rf_advance_offset(struct rf_parse_state *st, unsigned n);

/**
 * Read the header of a given file in a parser state.
 *
 * This function reset the parser's offset to the start of the file and then
 * read the content of the Runnner Format header.
 *
 * @warnning
 *  This function will reset the file offset back to zero before reading the
 *  header, so careful if reading the header after you have moved the offset.
 *
 * @param st    The current parser state in use.
 * @param hdr   The header to read into.
 *
 * @return 0 on success, -1 and sets st->error on failure.
 */
int rf_read_hdr(struct rf_parse_state *st, struct rf_hdr *hdr);

/**
 * Validate the content of a given rf_hdr header.
 *
 * @param hdr The RF header to validate.
 * @param st  The parser state to fill in (optional)
 *
 * @return 0 on success, -1 on error and set st->error.
 */
int rf_validate_hdr(struct rf_hdr *hdr, struct rf_parse_state *st);

/**
 * Read the next section in the file starting from the current offset.
 *
 * If the offset is zero, then it sets an error indicating that the header must
 * be read and validated at first.
 * If the header is invalid, then it will return and maintain the same error
 * state.
 *
 * @param st    The parser state from which to continue reading.
 * @param shdr  A section header to fill in while reading.
 * @param buf   A pointer to a buffer that rf_read_section will allocate.
 *
 * @return 0 on success with the buffer allocated, -1 on error and sets
 *  st->error.
 */
int rf_read_section(struct rf_parse_state *st, struct rf_shdr *shdr,
                    unsigned char **buf);

/**
 * Read a given section's header from the current file offset.
 *
 * If the offset is zero, then it sets an error indicating that the header must
 * be read and validated at first.
 * If the header is invalid, then it will return and maintain the same error
 * state.
 *
 * @param st  The parser state from which to continue reading.
 * @param shdr A section header to fill in while reading.
 *
 * @return 0 on success, -1 on error and set st->error.
 */
int rf_read_section_header(struct rf_parse_state *st, struct rf_shdr *shdr);

/**
 * Find a section in the file by name.
 *
 * @warning
 *  This function will reset the file offset to zero before starting and after
 *  it is done the lookup.
 *
 * @param st    The parser state from which to continue reading.
 * @param shdr  A section header to fill in if found.
 * @param name  The name of the section to look for.
 *
 * @return 0 on success, -1 on failure and sets st->error.
 */
int rf_find_section_by_name(struct rf_parse_state *st, struct rf_shdr *shdr,
                            const char *name);

/**
 * Read a section body given its header.
 *
 * @warning
 *  This function will modify the current file offset and reset it to zero once
 *  it is done.
 *
 * @param st    The parser state from which to continue reading.
 * @param shdr  The section header to read from.
 * @param buf   A pointer to a buffer that rf_read_section will allocate.
 *
 * @return 0 on success, -1 on failure and set st->error.
 */
int rf_read_section_body(struct rf_parse_state *st, struct rf_shdr *shdr,
                         unsigned char **buf);

/**
 * Return a string representation of the error.
 *
 * @param err  The parser error to print.
 *
 * @return A pointer to static string containing the string representation of
 * the error.
 */
const char *rf_strerror(enum RF_ERROR err);

#endif // rh_parse.h
