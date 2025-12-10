#ifndef _VMM_H
#define _VMM_H
//*****************************************************************************
//*
//*  @file		vmm.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief	    Defines the interface for the virtual memory manager (VMM).
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <mm/pde.h>
#include <mm/pte.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

#define VMM_PAGE_SIZE           4096    //! 4KB page size
#define VMM_PAGES_PER_TABLE     1024    //! 1024 entries per page table
#define VMM_PAGES_PER_DIR       1024    //! 1024 entries per page directory

//! dir index mask (last 10 bits of a 32 bit address)
#define VMM_DIR_INDEX_MASK      0xFFC00000
//! table index mask (next 10 bits of a 32 bit address) 
#define VMM_TABLE_INDEX_MASK    0x003FF000
//! page offset mask (last 12 bits of a 32 bit address)
#define VMM_PAGE_OFFSET_MASK    0x00000FFF

//! macros to retrieve the indices and offsets from a virtual address

//! Get directory index from address
#define VMM_DIR_INDEX(addr)     (((uintptr_t)(addr) & VMM_DIR_INDEX_MASK) >> 22) 
//! Get table index from address
#define VMM_TABLE_INDEX(addr)   (((uintptr_t)(addr) & VMM_TABLE_INDEX_MASK) >> 12) 
//! Get page offset from address
#define VMM_PAGE_OFFSET(addr)   (((uintptr_t)(addr) & VMM_PAGE_OFFSET_MASK))       

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! represents a single page table in memory
typedef struct {

    //! array of page table entries 
    pte_t       table[ VMM_PAGES_PER_TABLE ];

} pagetable_t;

//! represents a page directory
typedef struct {

    //! array of page directory entries
    pde_t       table[ VMM_PAGES_PER_DIR ];

} pagedir_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//! Initializes the kernel virtual memory manager.
void        vmm_init (void);

//! creates a new page table for the specified virtual address
void        vmm_create_pt (pagedir_t* pdir, void* virtual, uint32_t flags);

//! Maps a virtual address to a physical address in the page tables.
void        vmm_map_page ( pagedir_t* pdir, void* virtual, void* physical,
                           uint32_t flags );

//! Allocates a corresponding frame for the given page table entry.
int32_t     vmm_page_alloc (pte_t *pte, uint32_t flags);

//! Frees the frame associated with the given page table entry,
//! also marks the entry as not present.
void        vmm_page_free (pte_t *pte);

//! Allocates a region of virtual memory in the given page directory.
bool        vmm_alloc_region (pagedir_t* pdir, void* virtual, size_t size,
                              uint32_t flags);

//! Frees a region of virtual memory in the given page directory.
bool        vmm_free_region (pagedir_t* pdir, void* virtual, size_t size);

//! returns the physical address corresponding to the given virtual address
void*       vmm_get_phys_frame (pagedir_t* pdir, void* virtual);

//! returns the kernel page directory
pagedir_t*  vmm_get_kerneldir (void);

//! returns the current page directory
pagedir_t*  vmm_get_current_pagedir (void);

//! creates a new address space and returns a pointer to the new page directory
pagedir_t*  vmm_create_address_space ();

//! deep clones the given page table
pagetable_t* vmm_clone_pagetable (pagetable_t* src);

//! clones the current process' page directory
pagedir_t*  vmm_clone_pagedir (void);

//! Switch to a new page directory.
bool        vmm_switch_pagedir (pagedir_t* new_pagedir);

//! Destroys a page directory, freeing all user space page tables and frames.
void        vmm_destroy_pagedir (pagedir_t* pdir);

//*****************************************************************************
//**
//** 	END vmm.h
//**
//*****************************************************************************

#endif // _VMM_H