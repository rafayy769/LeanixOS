/* user.ld - Linker script for user-space programs */

/* entry point is main.
	TODO: add _start support */
ENTRY(main)

/* Load address: Start at 1MB (0x100000) to avoid kernel's reserved low memory
   User space has 1MB to 3GB available (0x100000 to 0xC0000000) */
/* MEMORY
{
    user_space : ORIGIN = 0x00100000, LENGTH = 0xBFF00000
}
 */
SECTIONS
{
    /* Program starts at 1MB */
    . = 0x00100000;

    /* Code sections */
    .text ALIGN(4K) : {
        *(.text)          /* Code */
        *(.text.*)
    }

	/* Read only data sections */
	.rodata ALIGN(4K) : {
		*(.rodata)
		*(.rodata.*)
	}

    /* Read-write data sections */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }

    /* BSS section (uninitialized data) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Mark end of program */
    _end = .;

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}