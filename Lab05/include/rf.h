/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifndef RF_H
#define RF_H

/**
 * The allowed section types in the Runner Format.
 */
enum RF_STYPE {
  RFS_NULL = 0, //!< A null section.
  RFS_CODE,     //!< A section containing code.
  RFS_DATA,     //!< A section containing process data.

  RFS_OTHER //!< Leave me at the end always.
};

//!< A readable RF section.
#define RF_FREAD 1
//!< A writable section.
#define RF_FWRITE 1 << 1
//!< An executable section.
#define RF_FEXEC 1 << 2

// Helper macros
#define RF_ISREADABLE(flag) (flag & RF_FREAD)
#define RF_ISWRITEABLE(flag) (flag & RF_FWRITE)
#define RF_ISEXEC(flag) (flag & RF_FEXEC)

//!< The name that each file must start with.
#define RF_HDR_NAME "CSSE332"
//!< The current version number.
#define RF_HDR_VERSION 1
//!< The magic number in the header.
#define RF_HDR_MAGIC "FORREST"

/**
 * The header of a CSSE332 Runner formatted executable.
 */
struct rf_hdr {
  unsigned char sig[16];      //!< 16 bytes signature which must be valid.
  unsigned num_sections;      //!< 4 bytes to represent the number of sections.
  unsigned sec_hdr_len;       //!< 4 bytes to represent a section header length.
  unsigned long entry_offset; //!< offset of the entry for this file.
} __attribute__((packed));

/**
 * A section header in a CSSE332 Runner formatted executable.
 */
struct rf_shdr {
  unsigned char name[16]; //!< At most 16 bytes for the section name.
  unsigned offset;        //!< The section body's offset in the file.
  unsigned len;           //!< The length of the section's body in bytes.
  unsigned type;          //!< The type for this section.
  unsigned flags;         //!< Any specific flags for the section.
  unsigned long addr;     //!< A hint to the address of this section in memory.
} __attribute__((packed));

#endif // rf.h
