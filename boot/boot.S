/**********************************************
 *
 * @file    boot.s
 * @author  Abdul Rafay (abdul.rafay@lums.edu.pk)
 *
 * @brief   Bootsector code for x86.
 *          This bootloader is written in 16-bit assembly and is designed to
 *          load the kernel into memory and switch to protected mode.
 * update:  updated to use preprocessor, for stage 1/stage2.
 *
**********************************************/

#include    "common.s"

#define     BOOTSECTOR_SIZE       0x200
#define     BOOTSECTOR_START      0x7c00
#define     BOOTSECTOR_MAGIC      0xAA55

//! in order to compile a 2 stage bootloader
#define     STAGE2

/* we can either load stage 2 or the kernel directly */
#ifndef     STAGE2

#define     _READ_SECTS           124    /* static for now
                                            124 is the max without crossing
                                            DMA buffer boundary */
#else

#define     _READ_SECTS           0x1    /* for stage2, just read next sector */

#define     STG2_LOAD_SEG         0x07c0
#define     STG2_LOAD_OFS         BOOTSECTOR_SIZE
#define     STG2_START            0x7e00

#endif

//! utilities defined in other modules

.extern     boot_print_line
.extern     get_bios_e820_mmap
.extern     get_bios_e801_memsize

#ifndef     STAGE2 
.extern     protected_start     /* directly init the protected mode
                                    no paging, only GDT based seg */
#else
.extern     stage2
#endif

.global     __bootsector_start

.code16

__bootsector_start:
    jmp     boot

//! includes the default GDT, laying out a flat memory map for the bootloader.
//!     note once the kernel is loaded correctly, this will be revisited

#include    "gdt.s"

boot:
    cli                        /* clear the interrupt flag */
    cld                        /* clear the direction flag (DF set to increment
                                    for string operations) */

    //! init setup for an early stack for procedure calls like printing etc.

    movw    $_STACK_TOP_REAL,  %bp
    movw    %bp,               %sp
    movw    $BOOTUP_MSG,       %bx         /* load the message address in bx */
    call    boot_print_line

    //! get the available memory size

    xor     %eax,              %eax
    xor     %ebx,              %ebx
    call    get_bios_e801_memsize

    //! query the memory size information from bios

    movw    %ax, (_MEMSIZE_LOW_LOC)     /* # of 1K blocks (1MB to 16MB) */
    movw    %bx, (_MEMSIZE_HIGH_LOC)    /* # of 64K blocks (above 16MB) */

    xorw    %ax,               %ax      /* clear ax */
    movw    %ax,               %es      /* set es to 0 (segment 0) */
    movw    $_MMAP_LOAD_LOC,   %di      /* di = load location */
    call    get_bios_e820_mmap          /* get the mmap (stored in es:di) */
    
    /* call the subroutine to load the kernel at the specified location */
    call    load_next

#ifndef     STAGE2
    //! just directly switch to protected mode and call the kernel
    jmp     switch_to_protected

#else
    //! jump to stage 2 loaded right after the bootsector
    jmp     stage2

#endif  // STAGE2

    //! should never be returning here
    hlt

/**********************************************
 *
 * load_next - Loads the kernel or stage2 from the disk (starting from
 *          sector 2, next to bootsector) into memory.
 * 
**********************************************/

load_next:

    movw    $KERNEL_LOAD_MSG,    %bx
    call    boot_print_string

#ifndef     STAGE2

    //! specifies the buffer to use for the kernel to load into

    movw    $_KERNEL_LOAD_SEG,      %ax  
    movw    %ax,                    %es  /* cant directly move to segment reg */
    movw    $_KERNEL_LOAD_OFFSET,   %bx  /* offset in es */
    // xorw    %bx,                %bx

#else  /* otherwise provide the offset and segment for stage2 */

    movw    $STG2_LOAD_SEG,     %ax
    movw    %ax,                %es
    movw    $STG2_LOAD_OFS,     %bx

#endif // STAGE2

    //! set up the registers for the disk read (int 0x13)

    movb    $0x2,               %ah        /* function select: read sectors */
    movb    $_READ_SECTS,       %al        /* read all sectors needed */
    movb    $0x0,               %ch        /* cylinder 0 */
    movb    $0x2,               %cl        /* sector # 2 */
    movb    $0x0,               %dh        /* head 0 */
    movb    $0x80,              %dl        /* drive 0x80 (hd1) */

    /* read the kernel from the disk and check for errors (ah=0x00 means success) */
    int     $0x13

    cmpb    $0x00,              %ah        /* check if the read was successful */
    jne     .error

    ret
    
    .error:
    hlt

//! switches to protected mode by modifying cr0 and loading the GDT

.global     switch_to_protected
switch_to_protected:

    xorw    %ax,               %ax
    movw    %ax,               %ds         /*  addr to gdt is %ds:gdt_desc */
    lgdt    (gdt_desc) 
    mov     %cr0,              %eax
    or      $0x1,              %eax        /* set the PE bit in CR0 to 1 */
    mov     %eax,              %cr0

    //!  jump to the protected mode code
    //!  implicitly updates the CS register to the code segment offset

    ljmp    $CODE_SEG_OFFSET,  $protected_start

//! protected start code is embedded in stage1 in both cases (32 bit code)

BOOTUP_MSG:                     .ascii "> leenix1.0 bootloader started...\0"
KERNEL_LOAD_MSG:                .ascii "> loading the kernel \0"
