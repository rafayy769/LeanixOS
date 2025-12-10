#ifndef _MM_PDE_H
#define _MM_PDE_H

#include <stdint.h>

// Page Directory Entry (PDE) flags
#define PDE_PRESENT         0x001       // Page is present in memory
#define PDE_WRITABLE        0x002       // Page is writable
#define PDE_USER            0x004       // Page is accessible in user mode
#define PDE_WRITETHROUGH    0x008       // Page Write-Through
#define PDE_CACHEDISABLE    0x010       // Page Cache Disable
#define PDE_ACCESSED        0x020       // Page has been accessed
#define PDE_DIRTY           0x040       // Page has been written to
#define PDE_SIZE_4MB        0x080       // Page size is 4MB
#define PDE_GLOBAL          0x100       // Page is global (not flushed on context switch)
#define PDE_LV4_GLOBAL      0x200       // Level 4 global page
#define PDE_FRAME_MASK      0xFFFFF000  // Mask for the frame address in the PDE

// Each Page Directory Entry is 32 bits wide, so we can represent it as a 32-bit unsigned integer
typedef uint32_t pde_t;

// Using a similar interface as PTEs for creating and manipulating PDEs
#define PDE_PTABLE_ADDR(pde)   ((pde) & PDE_FRAME_MASK)   // Get the page table address
#define PDE_FLAGS(pde)         ((pde) & ~PDE_FRAME_MASK)  // Get the flags from a PDE

#define PDE_IS_WRITABLE(pde)   ((pde) & PDE_WRITABLE)     // Check if the page is writable
#define PDE_IS_PRESENT(pde)    ((pde) & PDE_PRESENT)      // Check if the page is present
#define PDE_IS_DIRTY(pde)      ((pde) & PDE_DIRTY)        // Check if the page is dirty
#define PDE_IS_USER(pde)       ((pde) & PDE_USER)         // Check if the page is user accessible
#define PDE_IS_4MB(pde)        ((pde) & PDE_SIZE_4MB)     // Check if the page is 4MB

/**
 * @brief Sets the flags of a page directory entry.
 * 
 * @param pde Pointer to the page directory entry.
 * @param flags Flags to set in the page directory entry.
 */
static inline void pde_set_flags(pde_t *pde, uint32_t flags) {
    *pde = (*pde & PDE_FRAME_MASK) | (flags & ~PDE_FRAME_MASK);
}

/**
 * @brief Clears the specified flags of a page directory entry.
 * 
 * @param pde Pointer to the page directory entry.
 * @param flags Flags to clear in the page directory entry.
 */
static inline void pde_clear_flags(pde_t *pde, uint32_t flags) {
    *pde &= ~(flags & ~PDE_FRAME_MASK);
}

/**
 * @brief Creates a new page directory entry with the specified frame address and flags.
 * 
 * @param frame_addr The physical address of the frame.
 * @param flags Flags to set in the page directory entry.
 * @return pde_t The created page directory entry.
 */
static inline pde_t pde_create(void* frame_addr, uint32_t flags)
{
    return ((pde_t)frame_addr & PDE_FRAME_MASK) | (flags & ~PDE_FRAME_MASK);
}

#endif // _MM_PDE_H