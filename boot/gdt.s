
//!  The GDT is a table of segment descriptors that the processor uses to
//!     determine the attributes of memory segments. The table is made up
//!     of 8-byte entries, each of which describes a segment of memory. Its
//!     address is loaded into the GDTR register.
//!     currently we have 3 entries in the GDT: null, code, and data

.global     CODE_SEG_OFFSET
.global     DATA_SEG_OFFSET

.global     gdt_desc

GDT_ENTRY_SIZE      = 8

gdt_start:
    .zero   GDT_ENTRY_SIZE

gdt_code:
    .word   0xFFFF        /* limit of 4GB */
    .word   0x0000        /* base at 0 */
    .byte   0x00          /* base at 0 (Base 16:23 in the base address) */
    .byte   0b10011010    /* segment type (code, non-conforming, read) */
    .byte   0b11001111    /* (limit bits 16:19, granularity, 32-bit, 4KB) */
    .byte   0x00          /* base at 0 (Base 24:31 in the base address) */

gdt_data:
    .word   0xFFFF        /* limit of 4GB */
    .word   0x0000        /* base at 0 */
    .byte   0x00          /* base at 0 (Base 16:23 in the base address) */
    .byte   0b10010010    /* segment type (data, expand-down, read/write) */
    .byte   0b11001111    /* (limit bits 16:19, granularity, 32-bit, 4KB) */
    .byte   0x00          /* base at 0 (Base 24:31 in the base address) */

gdt_end:


//!  specifies the start of the GDT. can be used to load the GDT
//!     into the GDTR register.

gdt_desc:
    .word   gdt_end - gdt_start - 1     /* size of the GDT in bytes */
    .long   gdt_start                   /* address of the GDT (start) */


//! offsets for the code and data segments in the GDT
//!     these offsets are used to set the CS and DS registers in the CPU
//!     when switching to protected mode.

CODE_SEG_OFFSET = gdt_code - gdt_start
DATA_SEG_OFFSET = gdt_data - gdt_start
