// defines the early memory configuration defines

#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

/* The DMA buffer is used for DMA transfers on legacy devices, given the
	constraints for the buffer, we set it up in low memory. */
#define DMA_BUFFER_START 	  0x1000 // 4KB

/* the bootsector code sets up page tables so that physical memory starting
	at 0x0 can be accessed directly at 3GB and so on. */
#define PHYSMAP_BASE 		  0xC0000000

// we can then define some macros to get address translations
#define PHYS_TO_VIRT(addr)    ((void*) ((uintptr_t)(addr) + PHYSMAP_BASE))
#define VIRT_TO_PHYS(addr)    ((void*) ((uintptr_t)(addr) - PHYSMAP_BASE))

/* Kernel's physical and virtual address (if paging enabled with higher half) */
#define KERNEL_LOAD_PHYS   	  0x00100000 // 1MB
#define KERNEL_LOAD_VIRT   	  PHYS_TO_VIRT(KERNEL_LOAD_PHYS) // 3GB

/* low memory below 640 KB for initial kernel stack */
#define KERNEL_STACK_EARLY 	  0x00090000 // ~128KB stack space.

/* memory region for the kernel heap */
#define KERNEL_HEAP_VIRT   	  0xC0200000 // 3GB + 2MB
#define KERNEL_HEAP_SIZE   	  0x00100000 // 1MB

/* we keep the low 1MB identity mapped to enable easy access to legacy
	features such as DMA buffers or video memory */
#define IDENTITY_MAP_START   0x00000000 // 0
#define IDENTITY_MAP_END     0x00100000 // 1MB


/* memory for the user space programs */
#define USER_STACK_SIZE      0x1000
#define USER_STACK_TOP	   	 0xBFFFE000 // just below 3GB


#endif