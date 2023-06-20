ENTRY(Reset_Handler)

MEMORY {
       ROM       (RX)  : ORIGIN = 0x00000000 , LENGTH =   8*1024
       SRAM0     (RX)  : ORIGIN = 0x20000000 , LENGTH = 192*1024
       SRAM      (RWX) : ORIGIN = 0x20030000 , LENGTH =  64*1024
}

MCU_STACK_SIZE  = 1408;
MCU_MIN_HEAP    = 4096;
_min_stack      = MCU_STACK_SIZE;               /* minimum stack space to reserve for the user app */
_eheap          = ORIGIN(SRAM)+LENGTH(SRAM);    /* End of heap is end of RAM */

SECTIONS {
  .stack (NOLOAD): ALIGN(4)  
  {
    _sstack = .;
    . += MCU_STACK_SIZE;
    . = ALIGN(4);
    _estack = .;    
  } >SRAM

  /* This is the uninitialized data section */
  .bss (NOLOAD):ALIGN(4)  
  {
    _sbss = . ; /* exported for the startup function */
    *(.shbss)
    KEEP(*(.earlybss*))
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN (8);
    *(.ram.b)
    . = ALIGN(4);
    _ebss = . ; /* exported for the startup function */
  } >SRAM

  .text : ALIGN(0x100) 
  {
    _stext = . ;
    KEEP(*(.isr_vectors))
    . = ALIGN(4);
    /* CREATE_OBJECT_SYMBOLS */
    *(.text .text.*)
    _etext = . ;
  } >SRAM0
    
  .rodata : ALIGN (4) 
  {
    *(.rodata .rodata.*)
    
    . = ALIGN(4);
    
    KEEP(*(.init))
    
    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;
    
    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;
    
    . = ALIGN(4);
    KEEP(*(.fini))
    
    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;
    
    *(.init .init.*)
    *(.fini .fini.*)
    
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
    
    . = ALIGN (8);
    *(.rom)
    *(.rom.b)

     _sidata = . ; /* exported for the startup function */
  } >SRAM0

  /*
    this data is expected by the program to be in ram
    but we have to store it in the ROM otherwise it
    will get lost between resets, so the startup code
    has to copy it into RAM before the program starts
  */
  .data : ALIGN(8) 
  {
    _sdata = . ; /* exported for the startup function */
    KEEP(*(.jcr))
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    . = ALIGN (8);
    *(.ram)
    *(.ramfunc*)
     . = ALIGN(4);
    _edata = . ; /* exported for the startup function */
  } >SRAM AT>SRAM0

  .heap (NOLOAD): ALIGN(4) 
  {
    _sheap = .; /* exported for the startup function */
  } >SRAM

  /* Ensure we have enough space left for stack and heap */
  ASSERT(((_eheap - _sheap) > MCU_MIN_HEAP), "Insufficient space for heap!")

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }

  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) KEEP (*(.gnu.attributes)) }
  .note.gnu.arm.ident 0 : { *(.note.gnu.arm.ident) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) /* *(.note.gnu.build-id) */ }
}
