#!/usr/bin/env python

import struct
from elftools.elf.constants import SH_FLAGS
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section
from enum import IntEnum
import argparse


##
# CSSE332 Format
#
# struct rf_hdr {
#   unsigned char sig[16]; //!< 16 bytes signature which must be valid.
#   unsigned num_sections; //!< 4 bytes to represent the number of sections.
#   unsigned sec_hdr_len;  //!< 4 bytes to represent a section header length.
#   unsigned entry_offset; //!< offset of the entry for this file.
# } __attribute__((packed));
#
#  0 1 2 3 4 5 6   7
# [C S S E 3 3 2] [Version    ]
# [F O R R E S T] [\0         ]
# [num_sec] [sec_hdr_len      ]
# [entry_offset               ]
#
#
# struct rf_shdr {
#   unsigned char name[16]; //!< At most 16 bytes for the section name.
#   unsigned offset;        //!< The section body's offset in the file.
#   unsigned len;           //!< The length of the section's body in bytes.
#   unsigned type;          //!< The type for this section.
#   unsigned flags;         //!< Any specific flags for the section.
#   unsigned long addr;     //!< A hint to the address of this section in memory.
# } __attribute__((packed));
#  0 1 2 3  4 5 6 7
# [name ..
#                  ]
# [offset ][len    ]
# [type   ][flags  ]
# [ addr ..
#                  ]
#


class RF_STYPE(IntEnum):
  """Section type enum to match the source code."""

  RFS_NULL = 0
  RFS_CODE = 1
  RFS_DATA = 2
  RFS_BSS = 3
  RFS_OTHER = 4


class RF_SFLAGS(IntEnum):
  RF_FREAD = 1
  RF_FWRITE = 1 << 1
  RF_FEXEC = 1 << 2


class RF_Section:
  format = "<16sIIIIQ"

  def __init__(self, elfsec: Section):
    self.name = elfsec.name
    self.offset = elfsec["sh_offset"]
    self.len = elfsec.data_size
    self.data = None
    _flags = elfsec["sh_flags"]

    # Set the type, only two type either .text or .data
    if elfsec["sh_type"] == "SHT_PROGBITS":
      if _flags & SH_FLAGS.SHF_EXECINSTR:
        self.type = RF_STYPE.RFS_CODE
      else:
        self.type = RF_STYPE.RFS_DATA
    elif elfsec["sh_type"] == "SHT_NOBITS":
        self.type = RF_STYPE.RFS_DATA

    # set the flags, all start as readable.
    self.flags = RF_SFLAGS.RF_FREAD
    if _flags & SH_FLAGS.SHF_WRITE:
      self.flags = self.flags | RF_SFLAGS.RF_FWRITE
    if _flags & SH_FLAGS.SHF_EXECINSTR:
      self.flags = self.flags | RF_SFLAGS.RF_FEXEC

    # Save the data if it is contained.
    if elfsec.data_size > 0 and elfsec.data:
      self.data = elfsec.data()
    else:
      self.data = None

    # Check if we need to grab an address
    if elfsec["sh_addr"]:
      self.addr = elfsec["sh_addr"]
    else:
      self.addr = 0

  def pack_header(self):
    return struct.pack(
      self.format,
      self.name.encode('utf-8')[:16].ljust(16, b'\x00'),
      self.offset,
      self.len,
      self.type,
      self.flags,
      self.addr
    )


class RF_Header:
  format = "<7sB7sBIIQ"

  def __init__(self, num_sections):
    self.num_sections = num_sections
    self.sec_hdr_len = struct.calcsize(RF_Section.format)
    # note this offset is relative to body of the section in the ELF file.
    self.entry_offset = 0

  def build_signature(self):
    self.sig = b"CSSE332"
    self.version = 0x01
    self.magic = b"FORREST"
    self.eof = 0x00

  def pack(self):
    return struct.pack(
      self.format,
      self.sig,
      self.version,
      self.magic,
      self.eof,
      self.num_sections,
      self.sec_hdr_len,
      self.entry_offset,
    )


class RF_FILE:
  def __init__(self, ofile):
    self.sections = []
    self.ofile = ofile
    self.entry_offset = 0

  def from_elf(self, fname, sections):
    """
      Read an elf file and grab the desired sections.

      Args:
        fname:    The path to the input ELF file.
        sections: The sections to read out of the ELF file.

      Returns:
        None.
    """
    with open(fname, "rb") as f:
      elf = ELFFile(f)

      for sec in sections:
        elfsec = elf.get_section_by_name(sec)
        if elfsec and elfsec.data_size > 0:
          print(f"Added section {elfsec.name}")
          self.sections.append(RF_Section(elfsec))

      symtable = elf.get_section_by_name(".symtab")
      if symtable:
        for symbol in symtable.iter_symbols():
          if symbol.name == "start":
            self.entry_offset = symbol["st_value"]
            break

      self.rf_header = RF_Header(len(self.sections))
      self.rf_header.build_signature()
      self.rf_header.entry_offset = self.entry_offset

  def write_out(self):
    offset = 0
    with open(self.ofile, 'wb') as outfile:
      # write the header
      outfile.write(self.rf_header.pack())
      offset += struct.calcsize(RF_Header.format)

      # write each section afterwards
      for section in self.sections:
        # patch the offset to be for the current file.
        offset += struct.calcsize(RF_Section.format)
        print(f"Found {section.name}!")
        if section.data:
          section.offset = offset
          offset += len(section.data)

        outfile.write(section.pack_header())
        if section.data:
          outfile.write(section.data)

  def dump_stdout(self):
    print(f"Found {len(self.sections)} sections")
    print(f" Entry offset is at 0x{self.entry_offset:x}")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Generate Runner Format executable from object or executable ELF")
  parser.add_argument("-i", "--input", help="Input .c file")
  parser.add_argument("-o", "--output", default="a.rf", help="Output .rf file")
  args = parser.parse_args()

  rf_file = RF_FILE(args.output)
  rf_file.from_elf(args.input, [".text", ".data", ".bss", ".rodata"])
  rf_file.dump_stdout()
  rf_file.write_out()

