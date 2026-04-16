ENTRY(start)

PHDRS {
  text PT_LOAD FLAGS(5);
  data PT_LOAD FLAGS(6);
}

SECTIONS
{
  . = 0;

  .text : { *(.text) } :text

  . = ALIGN(0x1000);
  .data : { *(.data) } :data

  /DISCARD/ : {
    *(.comment)
    *(.note*)
    *(.eh_frame*)
    *(.plt*)
    *(.dynsym)
    *(.dynstr)
    *(.dynamic)
    *(.rela*)
    *(.hash)
    *(.gnu.hash)
  }
}
