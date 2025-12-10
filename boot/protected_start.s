/**********************************************
 *
 * @file    protected_start.s
 * @author  Abdul Rafay (abdul.rafay@lums.edu.pk)
 *
 * @brief   Protected mode code for x86. Note: relevant only for stage1.
 *          This code is executed after the bootloader has loaded the
 *          kernel into memory and switched to protected mode. It sets
 *          up the data segment registers, initializes the stack, and 
 *          jumps to the kernel entry point.
 *          Code in this file runs in 32 bit mode. we do a far
 *          jump to this code from the bootloader.
 *
*********************************************/

.code32

.include "common.s"     /* includes common definitions and constants */

KERNEL_LOAD_LOC         = _INIT_PHYS1

.global         protected_start

//! the entry point that calls main from the elf file
//!     uses segments from earlier set gdt

protected_start:

    //! set up the data segment registers

    movw $DATA_SEG_OFFSET, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    call    rebase_kernel

    //! set up the stack

    movl $_STACK_TOP_PROT, %ebp
    movl %ebp,             %esp

    //! jmp to the loaded ELF kernel entry point

    jmp *KERNEL_LOAD_LOC+ELF_ENTRY_OFFSET

    //! ideally should never reach here, but just in case

    hlt


//! moves the kernel from the initphys0 (64K) to initphys1 (1MB) location
rebase_kernel:

    movl  $_INIT_PHYS0, %esi
    movl  $_INIT_PHYS1, %edi
    movl  $KERNEL_SECTORS*512, %ecx  /* number of bytes to copy */
    rep movsb  /* copy the kernel from 64K to 1MB */

    ret
