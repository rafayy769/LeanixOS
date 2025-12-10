//! common defines for the bootloader 

//! keep this for loading the kernel directly at 0x100000 (1MB) (no rebase)

// _KERNEL_LOAD_SEG 	 = 0xffff
// _KERNEL_LOAD_OFFSET  = 0x0010 

//! keep this for loading the kernel at 0x10000 (64K)
//! works fine with stage2 as well, since kernel gets rebased to 1MB

_KERNEL_LOAD_SEG       = 0x1000   	/* segment number */
_KERNEL_LOAD_OFFSET    = 0X0      	/* segment offset */

//! memory locations for the bootloader to store memory size and mmap.
//! TODO: need to combine everything and provide a single struct
//!   called boot params to the kernel

_MEMSIZE_LOW_LOC          = 0x3000  /* 1K blocks between 1MB and 16MB */
_MEMSIZE_HIGH_LOC         = 0x3004  /* 64K blocks above 16MB */
_MMAP_ENTRIES_COUNT_LOC   = 0x3008  /* location to store the number 
										of e820 entries */
_MMAP_LOAD_LOC            = 0x300C  /* location to store the mmap */

//! kernel gets loaded at 0x10000 (64K) and then rebased to 0x100000 (1MB)
//!  in both single stage and 2 stage bootloaders.

_INIT_PHYS0 		= 0x010000		/* early physical location of kernel */
_INIT_PHYS1 		= 0x100000		/* kernel gets rebased here physically */
KERNEL_SECTORS      = 250    		/* number of sectors to read for the kernel */

ELF_ENTRY_OFFSET    = 0x18		/* defines the entry point for kmain 
									(extracted from elf hdr) */

//! protected mode and real mode stacks

_STACK_TOP_REAL 	= 0x07BFF		/* real mode stack top */
_STACK_TOP_PROT 	= 0x90000		/* protected mode stack top */

//! stage 2 bootloader sets up initial paging structures to enable higher
//!   half kernel.

_INIT_PDIR     		= 0x8000      	/* initial page directory location */
_INIT_IDENTMAP 		= 0x9000      	/* page table loc for identity map */
_INIT_PTABLE   		= 0x10000      	/* page table loc for High Half map */
_INIT_VIRTUAL  		= 0xc0000000  	/* kernel base address (virtual) */
