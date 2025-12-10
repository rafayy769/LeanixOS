//! This file contains the function to read and store the memory map read
//!  using the bios int 0x15, function 0xe820

.include    "common.s"

.code16
.global     get_bios_e820_mmap

/**********************************************
 *
 * get_bios_e820_mmap - 
 *      Read the memory map using BIOS interrupt 0x15, function 0xe820.
 *      Note: the function trashes all the register except %esi.
 *
 * @param %es:%di - Destination buffer to store 24 byte entries of the mem map.
 * @return %bp - Contains the number of entries read.
 *
*********************************************/

.type       get_bios_e820_mmap, @function
get_bios_e820_mmap:

    xorl   %ebx,         %ebx
    xorw   %bp,          %bp       /* null out %bp to hold the count */

    //!  configure the e820 call

    movl   $0x534D4150,  %edx      /* %edx = 'SMAP' */
    movw   $0xe820,      %ax       /* %ax = 0xe820, the func number */
    movl   $0x1,         20(%di)   /* force a valid acpi 3.x entry */
    mov    $24,          %ecx      /* %ecx = 24,the entry struct size */
    int    $0x15

    jc     .failure                /* 0xe820 unsupported if carry set */
    
    //! returned signature should be 'SMAP', edx gets trashed

    movl   $0x534D4150,  %edx      /* %edx ='SMAP' to ask again */
    cmp    %edx,         %eax      /* %eax == 'SMAP' on success */
    jne    .failure

    test   %ebx,         %ebx      /* ebx == 0 if >0 entries */
    je     .failure

    jmp    .tramp                  /* start processing */

    //! int 0x15, configured to read the memory map (eax = e820, edx = 'SMAP')
    //! main loop that reads the entries

    .e820loop:
    movw   $0xe820,      %ax
    movl   $0x1,         20(%di)
    mov    $24,          %ecx
    int    $0x15

    jc     .finish                 /* no more entries if carry set */

    movl   $0x534D4150,  %edx

    .tramp:
    jcxz   .skipentry              /* entry is empty if ecx == 0 */

    cmp    $20,          %cl       /* if cl <= 20 */
    jbe    .notext

    testb  $0x1,         20(%di)   /* check if the entry is valid (bit0) */
    je     .skipentry

    .notext:
    mov    8(%di),       %ecx      
    or     12(%di),      %ecx      
    jz     .skipentry

    inc    %bp                     /* increment the count of avail entries */
    add    $24,          %di       /* %di = address to put next entry */

    .skipentry:
    test   %ebx,         %ebx
    jne    .e820loop

    .finish:
    
    //! store the count at a well known address
    
    movw   %bp,          (_MMAP_ENTRIES_COUNT_LOC)

    clc                            # 
    ret

    .failure:
    stc
    ret
