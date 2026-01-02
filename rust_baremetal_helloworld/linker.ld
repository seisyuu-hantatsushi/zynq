ENTRY(_start)

MEMORY
{
  RAM (rwx) : ORIGIN = 0x00200000, LENGTH = 16M
}

SECTIONS
{
  .text : {
    KEEP(*(.text._start))
    *(.text .text.*)
    *(.rodata .rodata.*)
  } > RAM

  .data : ALIGN(4) {
    __sdata = .;
    *(.data .data.*)
    __edata = .;
  } > RAM

  .bss (NOLOAD) : ALIGN(4) {
    __sbss = .;
    *(.bss .bss.*)
    *(COMMON)
    __ebss = .;
  } > RAM

  . = ALIGN(8);
  __stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
