ENTRY(kmain);

SECTIONS
{
    
    . = 0xc0100000;
    kernel_start = .;       /* start of the kernel elf */
	
	. += 0x100;				/* to include the elf headers */

    kcode_start = .;		/* start of the kernel code */
    .text : ALIGN (0x100)   
                { *(.text) }
    kcode_end = .;			/* end of the kernel code */

    krodata_start = .;		/* start of the kernel rodata */
    .rodata :   { *(.rodata) *(.rodata.*) }
    krodata_end = .;		/* end of the kernel rodata */
    
    kdata_start = .;		/* start of the kernel data */
    .data :     { *(.data) }
    kdata_end = .;			/* end of the kernel data */

    kbss_start = .;		    /* start of the kernel bss */
    .bss  :     { *(.bss)  }
    kbss_end = .;			/* end of the kernel bss */

    /DISCARD/ : { *(.eh_frame .note.GNU-stack) }

    kernel_end = .;         /* End of the kernel elf */

}