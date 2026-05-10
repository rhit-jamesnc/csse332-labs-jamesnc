/**
 * Copyright (c) 2025 Rose-Hulman Institute of Technology. All Rights Reserved.
 *
 * @author <Your name>
 * @date   <Date last modified>
 */
#ifdef BUILD_BINUTILS
#include <bfd.h>
#include <dis-asm.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rf_parse.h"

#ifdef BUILD_BINUTILS
static int
styled_fprintf(void *stream, enum disassembler_style style, const char *fmt,
               ...)
{
  (void)style;
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(stream, fmt, args);
  va_end(args);
  return ret;
}

static void
disassemble_and_print(unsigned char *code, int len)
{
  struct disassemble_info info = {0};
  disassembler_ftype disasm    = {0};
  size_t pc                    = 0;
  size_t size                  = 0;

  bfd_init();
  init_disassemble_info(&info, stdout, (fprintf_ftype)fprintf, styled_fprintf);
  info.arch   = bfd_arch_i386;
  info.mach   = bfd_mach_x86_64;
  info.endian = BFD_ENDIAN_LITTLE;

  info.buffer               = code;
  info.buffer_vma           = 0;
  info.buffer_length        = len;
  info.read_memory_func     = buffer_read_memory;
  info.disassembler_options = NULL;
  info.insn_info_valid      = 0;

  disassemble_init_for_target(&info);

  disasm = disassembler(bfd_arch_i386, false, bfd_mach_x86_64, NULL);
  while(pc < len) {
    printf("0x%04lx: ", (unsigned long)pc);
    size = disasm(pc, &info);
    printf("\n");
    if(size <= 0)
      break;
    pc += size;
  }
}
#endif

int
main(int argc, char **argv)
{
  struct rf_parse_state st;
  struct rf_hdr hdr;
  struct rf_shdr shdr;
  unsigned char *buf;
  char signature[8] = {0};
  unsigned char version;

  printf("Reading the contents of the RF file in 3.f. Check out proc/3.c for "
         "the corresponding source code\n");

  // Load the parser
  if(rf_parse_state_init(&st, "3.rf")) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(st.error));
    exit(EXIT_FAILURE);
  }

  // Read the header
  if(rf_read_hdr(&st, &hdr)) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(st.error));
    rf_parse_state_destroy(&st);
    exit(EXIT_FAILURE);
  }

  // Validate the header
  if(rf_validate_hdr(&hdr, &st)) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(st.error));
    rf_parse_state_destroy(&st);
    exit(EXIT_FAILURE);
  }

  // Try the find-based approach
  if(rf_find_section_by_name(&st, &shdr, ".text")) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(st.error));
    rf_parse_state_destroy(&st);
    exit(EXIT_FAILURE);
  }

  // now fetch the body of that one
  if(rf_read_section_body(&st, &shdr, &buf)) {
    fprintf(stderr, "[ERROR]: %s\n", rf_strerror(st.error));
    rf_parse_state_destroy(&st);
    exit(EXIT_FAILURE);
  }

  // Print some information read from the file.
  memcpy(signature, hdr.sig, 7);
  version = hdr.sig[7];
  printf("*********************************************************************"
         "***********\n");
  printf("%-20s: %20u\n", "Number of sections", hdr.num_sections);
  printf("%-20s: %20p\n", "Entry offset", (void *)hdr.entry_offset);
  printf("%-20s: %20s\n", "Signature", signature);
  printf("%-20s: %20d\n", "Version", version);
  printf("%-20s: %20s\n", "Magic", (const char *)hdr.sig + 8);
  printf("*********************************************************************"
         "***********\n\n");

  printf("*********************************************************************"
         "***********\n");
  printf("%-20s: %20s\n", "Section name:", shdr.name);
  printf("%-20s: %20u\n", "Section offset:", shdr.offset);
  printf("%-20s: %20u\n", "Section length:", shdr.len);
  printf("%-20s: %20u\n", "Section type:", shdr.type);
  printf("%-20s: %20u\n", "Section flags:", shdr.flags);
  printf("%-20s: %20p\n", "Section address:", (void *)shdr.addr);
  printf("*********************************************************************"
         "***********\n\n");

#ifdef BUILD_BINUTILS
  disassemble_and_print(buf, shdr.len);
#endif

  // loop through remainig sections
  rf_read_section_header(&st, &shdr);
  while(st.error == RF_ERR_NULL) {
    if(strncmp((const char *)shdr.name, ".text", 16)) {
      // not .text, print it out too.
      printf("*****************************************************************"
             "****"
             "***********\n");
      printf("%-20s: %20s\n", "Section name:", shdr.name);
      printf("%-20s: %20u\n", "Section offset:", shdr.offset);
      printf("%-20s: %20u\n", "Section length:", shdr.len);
      printf("%-20s: %20u\n", "Section type:", shdr.type);
      printf("%-20s: %20u\n", "Section flags:", shdr.flags);
      printf("%-20s: %20p\n", "Section address:", (void *)shdr.addr);
      printf("*****************************************************************"
             "****"
             "***********\n\n");
    }

    // try again after moving the offset
    rf_advance_offset(&st, shdr.len);
    rf_read_section_header(&st, &shdr);
  }

  free(buf);
  rf_parse_state_destroy(&st);
  return 0;
}
