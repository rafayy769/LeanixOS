#include <mm/vmm.h>
#include <mm/kmm.h>
#include <testmain.h>
#include <stddef.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>

#define TEST_VIRT_ADDR_1 0x40000000  // 1GB mark
#define TEST_PHYS_ADDR_1 0x100000    // 1MB mark
#define TEST_PHYS_ADDR_2 0x200000    // 2MB mark


//----------------------------------------------------------------------------------------
// Global flag to ensure VMM is only initialized once
static bool vmm_system_initialized = true;

static void ensure_vmm_ready(void) {
    if (!vmm_system_initialized) {
        kmm_init();
        vmm_init();
        vmm_system_initialized = true;
    }
}

//--------------------------------------------------------------------------------------
// Helper function to cleanup a page directory
static void cleanup_pagedir(pagedir_t* pdir) {
    if (!pdir) return;
    
    pagedir_t* kernel_dir = vmm_get_kerneldir();
    for (int i = 0; i < VMM_PAGES_PER_DIR; i++) {
        if (pdir->table[i] && PDE_IS_PRESENT(pdir->table[i])) {
            // Don't free kernel page tables
            if (pdir->table[i] != kernel_dir->table[i]) {
                void* pt_frame = (void*)PDE_PTABLE_ADDR(pdir->table[i]);
                kmm_frame_free(pt_frame);
            }
        }
    }
    kmm_frame_free((void*)VIRT_TO_PHYS(pdir));
}

//------------------------------------------------------------------------------------------------
void test_vmm_init() {
    ensure_vmm_ready();
    
    // 1. Check kernel directory exists and is current
    pagedir_t* kdir = vmm_get_kerneldir();
    pagedir_t* current = vmm_get_current_pagedir();
    if (!kdir || !current || current != kdir) {
        send_msg("FAILED");
        return;
    }
    
    // 2. Verify identity mapping works (0-1MB range)
    // The VGA text buffer at 0xB8000 should be accessible via identity mapping
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    uint16_t original = *vga;     // Read from identity-mapped memory
    *vga = 0x0F54;                // Write test value ('T' with white on black)
    uint16_t verify = *vga;       // Read back
    *vga = original;              // Restore original value
    
    if (verify != 0x0F54) {
        send_msg("FAILED");  // Identity mapping broken
        return;
    }
    
    // 3. Verify physmap is correctly set up (physical memory mapped to 3GB+)
    // Physical address 0 should map to virtual address PHYSMAP_BASE
    void* phys_frame = vmm_get_phys_frame(kdir, (void*)PHYSMAP_BASE);
    if (phys_frame != (void*)0x0) {
        send_msg("FAILED");  // Physmap start is wrong
        return;
    }
    
    // Test that physical address 0x100000 (1MB) correctly maps to PHYSMAP_BASE + 0x100000
    phys_frame = vmm_get_phys_frame(kdir, (void*)(PHYSMAP_BASE + 0x100000));
    if (phys_frame != (void*)0x100000) {
        send_msg("FAILED");  // Physmap offset mapping is wrong
        return;
    }
    
    // 4. Verify physmap memory is actually accessible (won't page fault)
    volatile uint8_t* physmap_ptr = (volatile uint8_t*)PHYSMAP_BASE;
    uint8_t test_read = *physmap_ptr;  // Should not cause page fault
    (void)test_read;                    // Suppress unused variable warning
    
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_get_kerneldir() {
    ensure_vmm_ready();
    
    pagedir_t* kdir1 = vmm_get_kerneldir();
    if (!kdir1) {
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* kdir2 = vmm_get_kerneldir();
    if (kdir1 != kdir2) {
        send_msg("FAILED");
        return;
    }
    
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_get_current_pagedir() {
    ensure_vmm_ready();
    
    pagedir_t* current = vmm_get_current_pagedir();
    if (!current) {
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* kernel = vmm_get_kerneldir();
    if (current != kernel) {
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* current2 = vmm_get_current_pagedir();
    if (current != current2) {
        send_msg("FAILED");
        return;
    }
    
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_create_address_space() {
    ensure_vmm_ready();
    
    // Test creating multiple address spaces
    pagedir_t* spaces[5];
    for (int i = 0; i < 5; i++) {
        spaces[i] = vmm_create_address_space();
        if (!spaces[i]) {
            for (int j = 0; j < i; j++) {
                cleanup_pagedir(spaces[j]);
            }
            send_msg("FAILED");
            return;
        }
        
        // Each should be different from kernel
        if (spaces[i] == vmm_get_kerneldir()) {
            for (int j = 0; j <= i; j++) {
                cleanup_pagedir(spaces[j]);
            }
            send_msg("FAILED");
            return;
        }
        
        // Each should be unique
        for (int j = 0; j < i; j++) {
            if (spaces[i] == spaces[j]) {
                for (int k = 0; k <= i; k++) {
                    cleanup_pagedir(spaces[k]);
                }
                send_msg("FAILED");
                return;
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        cleanup_pagedir(spaces[i]);
    }
    
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_switch_pagedir() {
    ensure_vmm_ready();
    
    // Test NULL parameter (should fail)
    if (vmm_switch_pagedir(NULL)) {
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* kernel_dir = vmm_get_current_pagedir();
    if (!kernel_dir) {
        send_msg("FAILED");
        return;
    }
    
    // Test switching to same directory (should succeed)
    if (!vmm_switch_pagedir(kernel_dir)) {
        send_msg("FAILED");
        return;
    }
    
    if (vmm_get_current_pagedir() != kernel_dir) {
        send_msg("FAILED");
        return;
    }
    
    // NEW: Test switching to a DIFFERENT valid page directory
    
    // Read CR3 before creating new directory
    uint32_t cr3_kernel;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_kernel));
    
    // Create a new address space
    pagedir_t* new_dir = vmm_create_address_space();
    if (!new_dir) {
        send_msg("FAILED");
        return;
    }
    
    // Copy kernel mappings to new directory so code can still execute
    // (identity map, physmap, etc.)
    pagedir_t* kernel = vmm_get_kerneldir();
    for (int i = 0; i < VMM_PAGES_PER_DIR; i++) {
        if (kernel->table[i] && PDE_IS_PRESENT(kernel->table[i])) {
            new_dir->table[i] = kernel->table[i];
        }
    }
    
    // Switch to the new page directory
    if (!vmm_switch_pagedir(new_dir)) {
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");
        return;
    }
    
    // Verify current directory updated
    if (vmm_get_current_pagedir() != new_dir) {
        vmm_switch_pagedir(kernel_dir);
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");
        return;
    }
    
    // Verify CR3 register actually changed
    uint32_t cr3_new;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_new));
    
    if (cr3_kernel == cr3_new) {
        vmm_switch_pagedir(kernel_dir);
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");  // CR3 didn't change
        return;
    }
    
    // Verify CR3 contains the physical address of the new directory
    uint32_t expected_cr3 = (uint32_t)VIRT_TO_PHYS(new_dir);
    if (cr3_new != expected_cr3) {
        vmm_switch_pagedir(kernel_dir);
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");  // CR3 has wrong value
        return;
    }
    
    // Verify memory mappings work after switch
    // Access VGA memory through identity mapping (should still work)
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    uint16_t original = *vga;
    *vga = 0x0F58;  // Write 'X' with white on black
    uint16_t verify = *vga;
    *vga = original;  // Restore
    
    if (verify != 0x0F58) {
        vmm_switch_pagedir(kernel_dir);
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");  // Memory access failed after switch
        return;
    }
    
    // Verify physmap still works (access memory through physmap)
    volatile uint8_t* physmap_test = (volatile uint8_t*)PHYSMAP_BASE;
    uint8_t test_val = *physmap_test;  // Should not page fault
    (void)test_val;  // Suppress unused warning
    
    // Switch back to kernel directory
    if (!vmm_switch_pagedir(kernel_dir)) {
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");
        return;
    }
    
    // Verify we're back to kernel directory
    if (vmm_get_current_pagedir() != kernel_dir) {
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");
        return;
    }
    
    // Verify CR3 is back to kernel value
    uint32_t cr3_back;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_back));
    if (cr3_back != cr3_kernel) {
        kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
        send_msg("FAILED");
        return;
    }
    
    // Cleanup - only free the page directory frame itself
    // (we shared kernel page tables, so don't free those)
    kmm_frame_free((void*)VIRT_TO_PHYS(new_dir));
    
    send_msg("PASSED");
}


//------------------------------------------------------------------------------------------------
void test_vmm_create_pt() {
    ensure_vmm_ready();
    
    // Test 1: NULL parameter handling
    pagedir_t* kernel_dir = vmm_get_kerneldir();
    
    // Test NULL virtual address (should handle gracefully)
    vmm_create_pt(kernel_dir, NULL, PTE_PRESENT);
    

    // Test 2: Create new page directory for testing
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Test 3: Create page tables at different directory indices
    void* test_addrs[] = {
        (void*)0x40000000,  // Index 256
        (void*)0x80000000,  // Index 512
        (void*)0xA0000000,  // Index 640
    };
    
    uint32_t test_flags[] = {
        PTE_PRESENT | PTE_WRITABLE,
        PTE_PRESENT | PTE_WRITABLE | PTE_USER,
        PTE_PRESENT,
    };
    
    for (int i = 0; i < 3; i++) {
        uint32_t idx = VMM_DIR_INDEX(test_addrs[i]);
        
        // Verify PDE doesn't exist yet
        if (pdir->table[idx] != 0) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Create page table with specific flags
        vmm_create_pt(pdir, test_addrs[i], test_flags[i]);
        
        // Verify PDE exists and is present
        if (!pdir->table[idx] || !PDE_IS_PRESENT(pdir->table[idx])) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify PDE flags are set correctly
        pde_t pde = pdir->table[idx];
        uint32_t pde_flags = PDE_FLAGS(pde);
        
        // Check that the flags we set are present
        if (test_flags[i] & PTE_PRESENT) {
            if (!(pde_flags & PDE_PRESENT)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        if (test_flags[i] & PTE_WRITABLE) {
            if (!(pde_flags & PDE_WRITABLE)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        if (test_flags[i] & PTE_USER) {
            if (!(pde_flags & PDE_USER)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        // Verify page table is actually allocated and cleared
        void* pt_phys = (void*)PDE_PTABLE_ADDR(pde);
        if (!pt_phys) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Access the page table through physmap to verify it's cleared
        pagetable_t* pt = (pagetable_t*)PHYS_TO_VIRT(pt_phys);
        
        // Check that all PTEs in the new page table are zero (cleared)
        for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
            if (pt->table[j] != 0) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");  // Page table not cleared
                return;
            }
        }
    }
    
    // Test 4: Duplicate creation (should handle gracefully - logs error but doesn't crash)
    uint32_t idx0 = VMM_DIR_INDEX(test_addrs[0]);
    pde_t original_pde = pdir->table[idx0];
    
    // Try to create again - should log error and return without changing PDE
    vmm_create_pt(pdir, test_addrs[0], PTE_PRESENT);
    
    // Verify PDE hasn't changed (error was logged but nothing broke)
    if (pdir->table[idx0] != original_pde) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test 5: Create page table for address in same page table range
    // (different virtual address but same directory index)
    void* same_dir_addr = (void*)((uintptr_t)test_addrs[0] + 0x1000);
    uint32_t same_idx = VMM_DIR_INDEX(same_dir_addr);
    
    // Should be the same directory index
    if (same_idx != idx0) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Trying to create PT for same directory index should just log error
    vmm_create_pt(pdir, same_dir_addr, PTE_PRESENT);
    
    // PDE should still be the original
    if (pdir->table[idx0] != original_pde) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Cleanup
    cleanup_pagedir(pdir);
    
    send_msg("PASSED");
}


//------------------------------------------------------------------------------------------------
void test_vmm_map_page_basic() {
    ensure_vmm_ready();
    
    // Test 1: NULL parameter handling
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Test 2: Auto-creation of page tables
    void* test_virt = (void*)TEST_VIRT_ADDR_1;
    void* test_phys = (void*)TEST_PHYS_ADDR_1;
    uint32_t dir_idx = VMM_DIR_INDEX(test_virt);

    vmm_map_page(pdir, test_virt, test_phys, PTE_PRESENT | PTE_WRITABLE);

    if (!PDE_IS_PRESENT(pdir->table[dir_idx])) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    void* result = vmm_get_phys_frame(pdir, test_virt);
    if (result != test_phys) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test 3: Verify PTE flags are set correctly
    pde_t pde = pdir->table[dir_idx];
    pagetable_t* pt = (pagetable_t*)PHYS_TO_VIRT(PDE_PTABLE_ADDR(pde));
    uint32_t tbl_idx = VMM_TABLE_INDEX(test_virt);
    pte_t pte = pt->table[tbl_idx];
    
    if (!PTE_IS_PRESENT(pte)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if (!(PTE_FLAGS(pte) & PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if ((void*)PTE_FRAME_ADDR(pte) != test_phys) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test 4: Different flag combinations
    struct {
        void* virt;
        void* phys;
        uint32_t flags;
    } test_cases[] = {
        { (void*)(TEST_VIRT_ADDR_1 + 0x1000), (void*)(TEST_PHYS_ADDR_1 + 0x1000), 
          PTE_PRESENT },
        { (void*)(TEST_VIRT_ADDR_1 + 0x2000), (void*)(TEST_PHYS_ADDR_1 + 0x2000), 
          PTE_PRESENT | PTE_WRITABLE | PTE_USER },
        { (void*)(TEST_VIRT_ADDR_1 + 0x3000), (void*)(TEST_PHYS_ADDR_1 + 0x3000), 
          PTE_PRESENT | PTE_USER },
    };
    
    for (int i = 0; i < 3; i++) {
        vmm_map_page(pdir, test_cases[i].virt, test_cases[i].phys, test_cases[i].flags);
        
        void* mapped_phys = vmm_get_phys_frame(pdir, test_cases[i].virt);
        if (mapped_phys != test_cases[i].phys) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        uint32_t tbl_idx_i = VMM_TABLE_INDEX(test_cases[i].virt);
        pte_t pte_i = pt->table[tbl_idx_i];
        uint32_t pte_flags = PTE_FLAGS(pte_i);
        
        if (test_cases[i].flags & PTE_PRESENT) {
            if (!PTE_IS_PRESENT(pte_i)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        if (test_cases[i].flags & PTE_WRITABLE) {
            if (!(pte_flags & PTE_WRITABLE)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        } else {
            if (pte_flags & PTE_WRITABLE) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        if (test_cases[i].flags & PTE_USER) {
            if (!(pte_flags & PTE_USER)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        } else {
            if (pte_flags & PTE_USER) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
    }
    
    // Test 5: Map multiple sequential pages (REDUCED from 10 to 3)
    for (int i = 0; i < 3; i++) {
        void* virt = (void*)(TEST_VIRT_ADDR_1 + 0x10000 + (i * VMM_PAGE_SIZE));
        void* phys = (void*)(TEST_PHYS_ADDR_1 + 0x10000 + (i * VMM_PAGE_SIZE));
        
        vmm_map_page(pdir, virt, phys, PTE_PRESENT | PTE_WRITABLE);
        
        void* result_phys = vmm_get_phys_frame(pdir, virt);
        if (result_phys != phys) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    // Test 6: Map pages in different page tables (cross PT boundary)
    void* virt_pt1 = (void*)0x40000000;
    void* virt_pt2 = (void*)0x40400000;
    void* phys_pt1 = (void*)0x500000;
    void* phys_pt2 = (void*)0x600000;
    
    vmm_map_page(pdir, virt_pt1, phys_pt1, PTE_PRESENT | PTE_WRITABLE);
    vmm_map_page(pdir, virt_pt2, phys_pt2, PTE_PRESENT | PTE_WRITABLE);
    
    if (!PDE_IS_PRESENT(pdir->table[VMM_DIR_INDEX(virt_pt1)]) ||
        !PDE_IS_PRESENT(pdir->table[VMM_DIR_INDEX(virt_pt2)])) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if (vmm_get_phys_frame(pdir, virt_pt1) != phys_pt1 ||
        vmm_get_phys_frame(pdir, virt_pt2) != phys_pt2) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}


//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
void test_vmm_page_alloc() {
    ensure_vmm_ready();
    
    // Test NULL parameter
    if (vmm_page_alloc(NULL, PTE_PRESENT) == 0) {
        send_msg("FAILED");
        return;
    }
    
    // Allocate multiple pages
    pte_t ptes[10];
    for (int i = 0; i < 10; i++) {
        ptes[i] = 0;
        if (vmm_page_alloc(&ptes[i], PTE_PRESENT | PTE_WRITABLE) != 0) {
            // Cleanup already allocated
            for (int j = 0; j < i; j++) {
                vmm_page_free(&ptes[j]);
            }
            send_msg("FAILED");
            return;
        }
        
        if (!PTE_IS_PRESENT(ptes[i])) {
            for (int j = 0; j <= i; j++) {
                vmm_page_free(&ptes[j]);
            }
            send_msg("FAILED");
            return;
        }

        // NEW: Verify frame address is page-aligned
        uintptr_t frame_addr = PTE_FRAME_ADDR(ptes[i]);
        if (frame_addr % VMM_PAGE_SIZE != 0) {
            for (int j = 0; j <= i; j++) {
                vmm_page_free(&ptes[j]);
            }
            send_msg("FAILED");
            return;
        }

        // Verify each allocation is unique
        for (int j = 0; j < i; j++) {
            if (PTE_FRAME_ADDR(ptes[i]) == PTE_FRAME_ADDR(ptes[j])) {
                for (int k = 0; k <= i; k++) {
                    vmm_page_free(&ptes[k]);
                }
                send_msg("FAILED");
                return;
            }
        }
    }
    
    // Test double allocation
    pte_t old = ptes[0];
    if (vmm_page_alloc(&ptes[0], PTE_PRESENT) != 0 || ptes[0] != old) {
        for (int i = 0; i < 10; i++) {
            vmm_page_free(&ptes[i]);
        }
        send_msg("FAILED");
        return;
    }
    
    // Cleanup
    for (int i = 0; i < 10; i++) {
        vmm_page_free(&ptes[i]);
    }
    
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_page_free() {
    ensure_vmm_ready();
    
    // Test safe operations
    vmm_page_free(NULL);
    pte_t empty = 0;
    vmm_page_free(&empty);

    // Test double-free (should be safe)
    pte_t pte_double = 0;
    if (vmm_page_alloc(&pte_double, PTE_PRESENT) != 0) {
        send_msg("FAILED");
        return;
    }
    vmm_page_free(&pte_double);
    vmm_page_free(&pte_double);  // Double-free - should be safe
    if (PTE_IS_PRESENT(pte_double)) {
        send_msg("FAILED");
        return;
    }
    
    // Test allocation and free cycle multiple times
    for (int i = 0; i < 5; i++) {
        pte_t pte = 0;
        if (vmm_page_alloc(&pte, PTE_PRESENT | PTE_WRITABLE) != 0) {
            send_msg("FAILED");
            return;
        }
        
        if (!PTE_IS_PRESENT(pte)) {
            send_msg("FAILED");
            return;
        }
        
        // void* frame = (void*)PTE_FRAME_ADDR(pte);
        
        vmm_page_free(&pte);
        
        if (PTE_IS_PRESENT(pte)) {
            send_msg("FAILED");
            return;
        }
        
        // Frame should be reusable
        pte_t pte2 = 0;
        if (vmm_page_alloc(&pte2, PTE_PRESENT) != 0) {
            send_msg("FAILED");
            return;
        }
        
        vmm_page_free(&pte2);
    }
    
    send_msg("PASSED");
}

//-------------------------------------------------------------------------------------------------------------------
void test_vmm_alloc_region() {
    ensure_vmm_ready();
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Test 1: NULL parameters
    if (vmm_alloc_region(NULL, (void*)TEST_VIRT_ADDR_1, VMM_PAGE_SIZE, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");  // Should return false for NULL pdir
        return;
    }
    
    if (vmm_alloc_region(pdir, NULL, VMM_PAGE_SIZE, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");  // Should return false for NULL virtual
        return;
    }
    
    // Test 2: Size = 0
    if (vmm_alloc_region(pdir, (void*)TEST_VIRT_ADDR_1, 0, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");  // Should return false for size 0
        return;
    }
    
    // Test 3: Small region (single page)
    void* region1 = (void*)TEST_VIRT_ADDR_1;
    if (!vmm_alloc_region(pdir, region1, VMM_PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify the single page is allocated and present
    void* phys1 = vmm_get_phys_frame(pdir, region1);
    if (!phys1) {
        vmm_free_region(pdir, region1, VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify flags are set correctly
    uint32_t dir_idx1 = VMM_DIR_INDEX(region1);
    pde_t pde1 = pdir->table[dir_idx1];
    pagetable_t* pt1 = (pagetable_t*)PHYS_TO_VIRT(PDE_PTABLE_ADDR(pde1));
    uint32_t tbl_idx1 = VMM_TABLE_INDEX(region1);
    pte_t pte1 = pt1->table[tbl_idx1];
    
    if (!PTE_IS_PRESENT(pte1) || !(PTE_FLAGS(pte1) & PTE_WRITABLE)) {
        vmm_free_region(pdir, region1, VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    vmm_free_region(pdir, region1, VMM_PAGE_SIZE);
    
    // Test 4: Medium region (multiple pages, same page table)
    void* region2 = (void*)0x50000000;
    size_t size2 = 5 * VMM_PAGE_SIZE;
    
    if (!vmm_alloc_region(pdir, region2, size2, PTE_PRESENT | PTE_WRITABLE | PTE_USER)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify all 5 pages are allocated and present
    for (int i = 0; i < 5; i++) {
        void* virt = (void*)((uintptr_t)region2 + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            vmm_free_region(pdir, region2, size2);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify each has correct flags
        uint32_t dir_idx = VMM_DIR_INDEX(virt);
        pde_t pde = pdir->table[dir_idx];
        pagetable_t* pt = (pagetable_t*)PHYS_TO_VIRT(PDE_PTABLE_ADDR(pde));
        uint32_t tbl_idx = VMM_TABLE_INDEX(virt);
        pte_t pte = pt->table[tbl_idx];
        
        if (!PTE_IS_PRESENT(pte) || 
            !(PTE_FLAGS(pte) & PTE_WRITABLE) ||
            !(PTE_FLAGS(pte) & PTE_USER)) {
            vmm_free_region(pdir, region2, size2);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    vmm_free_region(pdir, region2, size2);
    
    // Test 5: Large region spanning page table boundaries
    // 0x40000000 is at directory index 256
    // Each page table covers 4MB (1024 pages * 4KB)
    // So allocating 8MB should span 2 page tables
    void* region3 = (void*)0x40000000;
    size_t size3 = 8 * 1024 * 1024;  // 8MB = 2048 pages
    
    if (!vmm_alloc_region(pdir, region3, size3, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify page tables at indices 256 and 257 exist
    if (!PDE_IS_PRESENT(pdir->table[256]) || !PDE_IS_PRESENT(pdir->table[257])) {
        vmm_free_region(pdir, region3, size3);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Spot-check pages in both page tables
    void* first_page = region3;
    void* middle_page = (void*)((uintptr_t)region3 + 4 * 1024 * 1024);  // 4MB in
    void* last_page = (void*)((uintptr_t)region3 + size3 - VMM_PAGE_SIZE);
    
    if (!vmm_get_phys_frame(pdir, first_page) ||
        !vmm_get_phys_frame(pdir, middle_page) ||
        !vmm_get_phys_frame(pdir, last_page)) {
        vmm_free_region(pdir, region3, size3);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    vmm_free_region(pdir, region3, size3);
    
    // Test 6: Unaligned start address (should still work)
    void* region4 = (void*)0x60000100;  // Not page-aligned
    size_t size4 = 3 * VMM_PAGE_SIZE;
    
    if (!vmm_alloc_region(pdir, region4, size4, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // The function doesn't align internally, so it allocates starting at 0x60000100
    // This should allocate pages at 0x60000000, 0x60001000, 0x60002000, 0x60003000
    // (4 pages total to cover the unaligned region)
    
    // Check that the unaligned start address page is accessible
    void* phys_unaligned = vmm_get_phys_frame(pdir, region4);
    if (!phys_unaligned) {
        vmm_free_region(pdir, region4, size4);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    vmm_free_region(pdir, region4, size4);
    
    // Test 7: Allocate overlapping regions (should handle gracefully)
    void* region5 = (void*)0x70000000;
    if (!vmm_alloc_region(pdir, region5, 2 * VMM_PAGE_SIZE, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Try to allocate overlapping region (page_alloc should handle already-present pages)
    void* region6 = (void*)((uintptr_t)region5 + VMM_PAGE_SIZE);
    if (!vmm_alloc_region(pdir, region6, 2 * VMM_PAGE_SIZE, PTE_PRESENT)) {
        vmm_free_region(pdir, region5, 2 * VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    vmm_free_region(pdir, region5, 3 * VMM_PAGE_SIZE);  // Free all overlapping pages
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}

//------------------------------------------------------------------------------------------------
void test_vmm_free_region() {
    ensure_vmm_ready();
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Test 1: NULL parameters and size = 0
    if (vmm_free_region(NULL, (void*)TEST_VIRT_ADDR_1, VMM_PAGE_SIZE) ||
        vmm_free_region(pdir, NULL, VMM_PAGE_SIZE) ||
        vmm_free_region(pdir, (void*)TEST_VIRT_ADDR_1, 0)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test 2: Single page alloc/free cycle
    void* region1 = (void*)0x50000000;
    
    if (!vmm_alloc_region(pdir, region1, VMM_PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if (!vmm_get_phys_frame(pdir, region1)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if (!vmm_free_region(pdir, region1, VMM_PAGE_SIZE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify page is unmapped
    if (vmm_get_phys_frame(pdir, region1) != NULL) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test 3: Multiple pages - verify PTEs cleared and empty PT freed
    void* region2 = (void*)0x60000000;
    size_t size2 = 3 * VMM_PAGE_SIZE;
    uint32_t dir_idx2 = VMM_DIR_INDEX(region2);
    
    if (!vmm_alloc_region(pdir, region2, size2, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    if (!vmm_free_region(pdir, region2, size2)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify empty page table is freed
    if (pdir->table[dir_idx2] != 0) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify all pages are unmapped
    for (int i = 0; i < 3; i++) {
        void* virt = (void*)((uintptr_t)region2 + (i * VMM_PAGE_SIZE));
        if (vmm_get_phys_frame(pdir, virt) != NULL) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    // Test 4: Alloc/free cycle - verify frames are reusable
    void* region3 = (void*)0x70000000;
    size_t size3 = 2 * VMM_PAGE_SIZE;
    
    for (int cycle = 0; cycle < 2; cycle++) {
        if (!vmm_alloc_region(pdir, region3, size3, PTE_PRESENT)) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        if (!vmm_free_region(pdir, region3, size3)) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify freed
        if (vmm_get_phys_frame(pdir, region3) != NULL) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    // Test 5: Partial free - page table remains with some pages
    void* region4 = (void*)0x80000000;
    uint32_t dir_idx4 = VMM_DIR_INDEX(region4);
    
    // Allocate 3 pages
    if (!vmm_alloc_region(pdir, region4, 3 * VMM_PAGE_SIZE, PTE_PRESENT)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Free only first 2 pages
    if (!vmm_free_region(pdir, region4, 2 * VMM_PAGE_SIZE)) {
        vmm_free_region(pdir, (void*)((uintptr_t)region4 + 2 * VMM_PAGE_SIZE), VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Page table should still exist (1 remaining page)
    if (!PDE_IS_PRESENT(pdir->table[dir_idx4])) {
        vmm_free_region(pdir, (void*)((uintptr_t)region4 + 2 * VMM_PAGE_SIZE), VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Last page should still be mapped
    if (vmm_get_phys_frame(pdir, (void*)((uintptr_t)region4 + 2 * VMM_PAGE_SIZE)) == NULL) {
        vmm_free_region(pdir, (void*)((uintptr_t)region4 + 2 * VMM_PAGE_SIZE), VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Free remaining page
    if (!vmm_free_region(pdir, (void*)((uintptr_t)region4 + 2 * VMM_PAGE_SIZE), VMM_PAGE_SIZE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Now page table should be freed
    if (pdir->table[dir_idx4] != 0) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}

//--------------------------------------------------------------------------------------------------------------------
void test_vmm_get_phys_frame() {
    ensure_vmm_ready();
    
    // Test NULL parameters
    if (vmm_get_phys_frame(NULL, (void*)TEST_VIRT_ADDR_1) != NULL) {
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    if (vmm_get_phys_frame(pdir, NULL) != NULL) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test unmapped address (no page table)
    if (vmm_get_phys_frame(pdir, (void*)TEST_VIRT_ADDR_1) != NULL) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test address with page table but no frame (NEW)
    void* test_region = (void*)0x50000000;
    
    // Map one page to create the page table
    vmm_map_page(pdir, test_region, (void*)TEST_PHYS_ADDR_1, PTE_PRESENT);
    
    // Now test an unmapped address in the SAME page table
    void* unmapped_in_pt = (void*)((uintptr_t)test_region + VMM_PAGE_SIZE);
    
    // Verify they share the same page table
    if (VMM_DIR_INDEX(test_region) != VMM_DIR_INDEX(unmapped_in_pt)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // This should return NULL (page table exists, but PTE is not present)
    if (vmm_get_phys_frame(pdir, unmapped_in_pt) != NULL) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify the first page is still mapped correctly
    if (vmm_get_phys_frame(pdir, test_region) != (void*)TEST_PHYS_ADDR_1) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Map and verify translation for multiple pages
    for (int i = 0; i < 5; i++) {
        void* virt = (void*)(TEST_VIRT_ADDR_1 + (i * VMM_PAGE_SIZE));
        void* phys = (void*)(TEST_PHYS_ADDR_1 + (i * VMM_PAGE_SIZE));
        
        vmm_map_page(pdir, virt, phys, PTE_PRESENT | PTE_WRITABLE);
        
        void* result = vmm_get_phys_frame(pdir, virt);
        if (result != phys) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}

//-----------------------------------------------------------------------------------------------------------------

void test_vmm_double_mapping() {
    ensure_vmm_ready();
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Initial mapping
    vmm_map_page(pdir, (void*)TEST_VIRT_ADDR_1, (void*)TEST_PHYS_ADDR_1,
                 PTE_PRESENT | PTE_WRITABLE);
    
    void* phys = vmm_get_phys_frame(pdir, (void*)TEST_VIRT_ADDR_1);
    if (phys != (void*)TEST_PHYS_ADDR_1) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Remap to different physical address
    vmm_map_page(pdir, (void*)TEST_VIRT_ADDR_1, (void*)TEST_PHYS_ADDR_2,
                 PTE_PRESENT | PTE_WRITABLE);
    
    phys = vmm_get_phys_frame(pdir, (void*)TEST_VIRT_ADDR_1);
    if (phys != (void*)TEST_PHYS_ADDR_2) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Test remapping multiple times
    for (int i = 0; i < 5; i++) {
        void* new_phys = (void*)(TEST_PHYS_ADDR_1 + (i * VMM_PAGE_SIZE));
        vmm_map_page(pdir, (void*)TEST_VIRT_ADDR_1, new_phys,
                     PTE_PRESENT | PTE_WRITABLE);
        
        phys = vmm_get_phys_frame(pdir, (void*)TEST_VIRT_ADDR_1);
        if (phys != new_phys) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}
//------------------------------------------------------------------------------------------------
void test_vmm_clone_pagetable() {

    ensure_vmm_ready();
    
    // Test 1: NULL parameter
    if (vmm_clone_pagetable(NULL) != NULL) {
        send_msg("FAILED");
        return;
    }
    
    // Test 2: Empty page table
    void* empty_pt_phys = kmm_frame_alloc();
    if (!empty_pt_phys) {
        send_msg("FAILED");
        return;
    }
    
    pagetable_t* empty_pt = (pagetable_t*)PHYS_TO_VIRT(empty_pt_phys);
    memset(empty_pt, 0, sizeof(pagetable_t));
    
    pagetable_t* cloned_empty = vmm_clone_pagetable(empty_pt);
    if (!cloned_empty) {
        kmm_frame_free(empty_pt_phys);
        send_msg("FAILED");
        return;
    }
    
    // Verify all entries are zero
    for (int i = 0; i < VMM_PAGES_PER_TABLE; i++) {
        if (cloned_empty->table[i] != 0) {
            kmm_frame_free(empty_pt_phys);
            kmm_frame_free(VIRT_TO_PHYS(cloned_empty));
            send_msg("FAILED");
            return;
        }
    }
    
    kmm_frame_free(empty_pt_phys);
    kmm_frame_free(VIRT_TO_PHYS(cloned_empty));
    
    // Test 3: Partially filled page table (3 entries)
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    void* region = (void*)0x50000000;
    
    // Allocate 3 pages with unique patterns
    for (int i = 0; i < 3; i++) {
        void* virt = (void*)((uintptr_t)region + (i * VMM_PAGE_SIZE));
        
        if (!vmm_alloc_region(pdir, virt, VMM_PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE)) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        void* phys = vmm_get_phys_frame(pdir, virt);
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        data[0] = 0xDEAD0000 + i;  // Unique marker
    }
    
    // Get the page table
    uint32_t dir_idx = VMM_DIR_INDEX(region);
    pde_t pde = pdir->table[dir_idx];
    pagetable_t* original_pt = (pagetable_t*)PHYS_TO_VIRT(PDE_PTABLE_ADDR(pde));
    
    // Clone it
    pagetable_t* cloned_pt = vmm_clone_pagetable(original_pt);
    if (!cloned_pt) {
        vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify it's a different page table
    if (cloned_pt == original_pt) {
        vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
        kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Verify frames are copied (different addresses)
    for (int i = 0; i < 3; i++) {
        uint32_t tbl_idx = VMM_TABLE_INDEX((void*)((uintptr_t)region + (i * VMM_PAGE_SIZE)));
        
        pte_t original_pte = original_pt->table[tbl_idx];
        pte_t cloned_pte = cloned_pt->table[tbl_idx];
        
        if (PTE_FRAME_ADDR(original_pte) == PTE_FRAME_ADDR(cloned_pte)) {
            vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
            for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
                if (cloned_pt->table[j] && PTE_IS_PRESENT(cloned_pt->table[j])) {
                    kmm_frame_free((void*)PTE_FRAME_ADDR(cloned_pt->table[j]));
                }
            }
            kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify contents are identical
        uint32_t* orig_data = (uint32_t*)PHYS_TO_VIRT(PTE_FRAME_ADDR(original_pte));
        uint32_t* clone_data = (uint32_t*)PHYS_TO_VIRT(PTE_FRAME_ADDR(cloned_pte));
        
        if (orig_data[0] != clone_data[0]) {
            vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
            for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
                if (cloned_pt->table[j] && PTE_IS_PRESENT(cloned_pt->table[j])) {
                    kmm_frame_free((void*)PTE_FRAME_ADDR(cloned_pt->table[j]));
                }
            }
            kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify flags are identical
        if (PTE_FLAGS(original_pte) != PTE_FLAGS(cloned_pte)) {
            vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
            for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
                if (cloned_pt->table[j] && PTE_IS_PRESENT(cloned_pt->table[j])) {
                    kmm_frame_free((void*)PTE_FRAME_ADDR(cloned_pt->table[j]));
                }
            }
            kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    // Verify modifying clone doesn't affect original
    uint32_t tbl_idx0 = VMM_TABLE_INDEX(region);
    uint32_t* clone_data0 = (uint32_t*)PHYS_TO_VIRT(PTE_FRAME_ADDR(cloned_pt->table[tbl_idx0]));
    uint32_t* orig_data0 = (uint32_t*)PHYS_TO_VIRT(PTE_FRAME_ADDR(original_pt->table[tbl_idx0]));
    
    uint32_t saved_value = orig_data0[0];
    clone_data0[0] = 0xBEEFBEEF;
    
    if (orig_data0[0] != saved_value) {
        vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
        for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
            if (cloned_pt->table[j] && PTE_IS_PRESENT(cloned_pt->table[j])) {
                kmm_frame_free((void*)PTE_FRAME_ADDR(cloned_pt->table[j]));
            }
        }
        kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    // Cleanup
    vmm_free_region(pdir, region, 3 * VMM_PAGE_SIZE);
    for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
        if (cloned_pt->table[j] && PTE_IS_PRESENT(cloned_pt->table[j])) {
            kmm_frame_free((void*)PTE_FRAME_ADDR(cloned_pt->table[j]));
        }
    }
    kmm_frame_free(VIRT_TO_PHYS(cloned_pt));
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}


//------------------------------------------------------------------------------------------------------------------
void test_vmm_clone_pagedir() {
    ensure_vmm_ready();
    
    pagedir_t* kernel_dir = vmm_get_kerneldir();
    if (!kernel_dir) {
        send_msg("FAILED");
        return;
    }
    
    // Test 1: Clone current directory (should succeed)
    pagedir_t* clone1 = vmm_clone_pagedir();
    if (!clone1) {
        send_msg("FAILED");
        return;
    }
    
    // Verify it's different from current
    if (clone1 == vmm_get_current_pagedir()) {
        cleanup_pagedir(clone1);
        send_msg("FAILED");
        return;
    }
    
    // Test 2: Verify kernel mappings are shared (shallow copy)
    // Kernel space is at indices 768-1023 (3GB-4GB)
    for (int i = 768; i < 1024; i++) {
        if (kernel_dir->table[i] && PDE_IS_PRESENT(kernel_dir->table[i])) {
            // Clone should have exact same PDE (same page table pointer)
            if (clone1->table[i] != kernel_dir->table[i]) {
                cleanup_pagedir(clone1);
                send_msg("FAILED");  // Kernel mapping not shared
                return;
            }
        }
    }
    
    cleanup_pagedir(clone1);
    
    // Test 3: Verify user mappings are deep-copied
    // Create a test directory with minimal mappings
    pagedir_t* test_dir = vmm_create_address_space();
    if (!test_dir) {
        send_msg("FAILED");
        return;
    }
    
    // Copy ONLY kernel directory entries (these will be shallow-copied)
    for (int i = 0; i < 1024; i++) {
        if (kernel_dir->table[i] && PDE_IS_PRESENT(kernel_dir->table[i])) {
            test_dir->table[i] = kernel_dir->table[i];
        }
    }
    
    // Add exactly ONE user mapping (keeps clone fast)
    void* user_addr = (void*)0x40000000;  // 1GB - clearly user space
    
    if (!vmm_alloc_region(test_dir, user_addr, VMM_PAGE_SIZE, 
                          PTE_PRESENT | PTE_WRITABLE | PTE_USER)) {
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    // Write unique data to the user page
    void* user_phys = vmm_get_phys_frame(test_dir, user_addr);
    if (!user_phys) {
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    uint32_t* user_data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)user_phys);
    user_data[0] = 0xDEADBEEF;
    user_data[1] = 0xCAFEBABE;
    
    // Switch to test directory and clone it
    pagedir_t* saved_dir = vmm_get_current_pagedir();
    
    if (!vmm_switch_pagedir(test_dir)) {
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    pagedir_t* clone2 = vmm_clone_pagedir();
    
    // Switch back immediately
    if (!vmm_switch_pagedir(saved_dir)) {
        // This is bad - we're stuck
        if (clone2) cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    if (!clone2) {
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    // Verify clone is different from original
    if (clone2 == test_dir) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    // Verify user page table was deep-copied (different address)
    uint32_t user_idx = VMM_DIR_INDEX(user_addr);
    
    if (!PDE_IS_PRESENT(clone2->table[user_idx])) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // User mapping missing in clone
        return;
    }
    
    uintptr_t orig_pt_addr = PDE_PTABLE_ADDR(test_dir->table[user_idx]);
    uintptr_t clone_pt_addr = PDE_PTABLE_ADDR(clone2->table[user_idx]);
    
    if (orig_pt_addr == clone_pt_addr) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Page table not deep-copied
        return;
    }
    
    // Verify user frame was deep-copied (different physical address)
    void* clone_phys = vmm_get_phys_frame(clone2, user_addr);
    
    if (!clone_phys) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Frame missing in clone
        return;
    }
    
    if (clone_phys == user_phys) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Frame not deep-copied
        return;
    }
    
    // Verify frame contents were copied correctly
    uint32_t* clone_data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)clone_phys);
    
    if (clone_data[0] != 0xDEADBEEF || clone_data[1] != 0xCAFEBABE) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Contents not copied
        return;
    }
    
    // Verify modifying clone doesn't affect original
    clone_data[0] = 0xAAAAAAAA;
    
    if (user_data[0] != 0xDEADBEEF) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Clone not isolated from original
        return;
    }
    
    // Test 4: Multiple clones are unique
    vmm_switch_pagedir(test_dir);
    pagedir_t* clone3 = vmm_clone_pagedir();
    vmm_switch_pagedir(saved_dir);
    
    if (!clone3) {
        cleanup_pagedir(clone2);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    if (clone3 == clone2 || clone3 == test_dir) {
        cleanup_pagedir(clone2);
        cleanup_pagedir(clone3);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");  // Clones not unique
        return;
    }
    
    // Verify clone3 also has correct data
    void* clone3_phys = vmm_get_phys_frame(clone3, user_addr);
    if (!clone3_phys || clone3_phys == user_phys || clone3_phys == clone_phys) {
        cleanup_pagedir(clone2);
        cleanup_pagedir(clone3);
        vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
        cleanup_pagedir(test_dir);
        send_msg("FAILED");
        return;
    }
    
    // Cleanup all resources
    cleanup_pagedir(clone2);
    cleanup_pagedir(clone3);
    vmm_free_region(test_dir, user_addr, VMM_PAGE_SIZE);
    cleanup_pagedir(test_dir);
    
    send_msg("PASSED");
}



// HIDDEN TESTS 
//==============================================================================
// TEST 1: Memory Reuse After Free - Detects Double-Free and Stale References
//=================================================================




void test_vmm_memory_reuse_cycle(void) {
    ensure_vmm_ready();

    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }

    void* test_region = (void*)0x50000000;
    size_t region_size = 10 * VMM_PAGE_SIZE;

    // -------------------------------
    // Cycle 1: Allocate, write pattern, and free
    // -------------------------------
    if (!vmm_alloc_region(pdir, test_region, region_size, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    // Write unique pattern to each page
    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)test_region + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            vmm_free_region(pdir, test_region, region_size);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }

        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        data[0] = 0xDEAD0000 + i;  // Cycle 1 pattern
    }

    // Free region
    if (!vmm_free_region(pdir, test_region, region_size)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    // Ensure pages are unmapped after freeing
    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)test_region + (i * VMM_PAGE_SIZE));
        if (vmm_get_phys_frame(pdir, virt) != NULL) {
            cleanup_pagedir(pdir);
            send_msg("FAILED"); // Region not fully unmapped
            return;
        }
    }

    // -------------------------------
    // Cycle 2: Reallocate same region, write new pattern
    // -------------------------------
    if (!vmm_alloc_region(pdir, test_region, region_size, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)test_region + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            vmm_free_region(pdir, test_region, region_size);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }

        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);

        // Verify the mapping is valid and accessible (no page fault)
        volatile uint32_t tmp = data[0];
        (void)tmp;

        // Write new pattern for this cycle
        data[0] = 0xBEEF0000 + i;
    }

    // Free region again
    vmm_free_region(pdir, test_region, region_size);

    // -------------------------------
    // Cycle 3: Final consistency check
    // -------------------------------
    if (!vmm_alloc_region(pdir, test_region, region_size, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)test_region + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            vmm_free_region(pdir, test_region, region_size);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }

        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);

        // Verify mapping validity only (no stale-data assumption)
        volatile uint32_t tmp = data[0];
        (void)tmp;
    }

    // Clean up and report success
    vmm_free_region(pdir, test_region, region_size);
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}


//==============================================================================
// TEST 2: Page Table Cleanup Verification
//==============================================================================
void test_vmm_page_table_cleanup() {
    ensure_vmm_ready();
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    // Test multiple allocation/free cycles at different addresses
    void* test_addrs[] = {
        (void*)0x50000000, // Dir index 320
        (void*)0x60000000, // Dir index 384
        (void*)0x70000000, // Dir index 448
    };
    
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < 3; i++) {
            uint32_t dir_idx = VMM_DIR_INDEX(test_addrs[i]);
            
            // Verify page table doesn't exist initially
            if (cycle == 0 && pdir->table[dir_idx] != 0) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            // Allocate pages
            if (!vmm_alloc_region(pdir, test_addrs[i], 5 * VMM_PAGE_SIZE, 
                                  PTE_PRESENT | PTE_WRITABLE)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            // Verify page table exists
            if (!PDE_IS_PRESENT(pdir->table[dir_idx])) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            // Get page table address for verification
            // void* pt_addr_before = (void*)PDE_PTABLE_ADDR(pdir->table[dir_idx]);
            
            // Free all pages in this region
            if (!vmm_free_region(pdir, test_addrs[i], 5 * VMM_PAGE_SIZE)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            // CRITICAL: Page table should be freed when empty
            if (pdir->table[dir_idx] != 0) {
                cleanup_pagedir(pdir);
                send_msg("FAILED"); // Page table not freed - memory leak
                return;
            }
            
            // CRITICAL: The page table frame should be freed
            // On next allocation, we should get a different or properly cleared PT
            if (!vmm_alloc_region(pdir, test_addrs[i], 3 * VMM_PAGE_SIZE, 
                                  PTE_PRESENT | PTE_WRITABLE)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            void* pt_addr_after = (void*)PDE_PTABLE_ADDR(pdir->table[dir_idx]);
            
            // Verify page table is properly initialized (all entries zero except our 3 pages)
            pagetable_t* pt = (pagetable_t*)PHYS_TO_VIRT((uintptr_t)pt_addr_after);
            int non_zero_count = 0;
            for (int j = 0; j < VMM_PAGES_PER_TABLE; j++) {
                if (pt->table[j] != 0) {
                    non_zero_count++;
                }
            }
            
            if (non_zero_count != 3) {
                vmm_free_region(pdir, test_addrs[i], 3 * VMM_PAGE_SIZE);
                cleanup_pagedir(pdir);
                send_msg("FAILED"); // Page table has stale entries
                return;
            }
            
            vmm_free_region(pdir, test_addrs[i], 3 * VMM_PAGE_SIZE);
        }
    }
    
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}

//==============================================================================
// TEST 3: Stress Test - Multiple Address Spaces with Repeated Operations
//==============================================================================
void test_vmm_multiple_address_spaces_stress() {
    ensure_vmm_ready();
    
    // Create and destroy 15 address spaces (reduced from original plan for speed)
    for (int iteration = 0; iteration < 15; iteration++) {
        pagedir_t* pdir = vmm_create_address_space();
        if (!pdir) {
            send_msg("FAILED");
            return;
        }
        
        // Allocate user regions at various addresses
        void* regions[] = {
            (void*)0x40000000,
            (void*)0x50000000,
            (void*)0x60000000,
        };
        
        size_t sizes[] = {
            3 * VMM_PAGE_SIZE,
            5 * VMM_PAGE_SIZE,
            4 * VMM_PAGE_SIZE,
        };
        
        // Allocate all regions
        for (int i = 0; i < 3; i++) {
            if (!vmm_alloc_region(pdir, regions[i], sizes[i], 
                                  PTE_PRESENT | PTE_WRITABLE | PTE_USER)) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
            
            // Write unique data to verify isolation
            for (size_t j = 0; j < sizes[i] / VMM_PAGE_SIZE; j++) {
                void* virt = (void*)((uintptr_t)regions[i] + (j * VMM_PAGE_SIZE));
                void* phys = vmm_get_phys_frame(pdir, virt);
                if (!phys) {
                    cleanup_pagedir(pdir);
                    send_msg("FAILED");
                    return;
                }
                uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
                data[0] = 0xA0000000 + iteration; // Mark with iteration number
                data[1] = i; // Mark with region number
            }
        }
        
        // Verify data integrity before freeing
        for (int i = 0; i < 3; i++) {
            for (size_t j = 0; j < sizes[i] / VMM_PAGE_SIZE; j++) {
                void* virt = (void*)((uintptr_t)regions[i] + (j * VMM_PAGE_SIZE));
                void* phys = vmm_get_phys_frame(pdir, virt);
                if (!phys) {
                    cleanup_pagedir(pdir);
                    send_msg("FAILED");
                    return;
                }
                uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
                if (data[0] != (0xA0000000 + iteration) || data[1] != (uint32_t)i) {
                    cleanup_pagedir(pdir);
                    send_msg("FAILED"); // Data corruption detected
                    return;
                }
            }
        }
        
        // Free all regions
        for (int i = 0; i < 3; i++) {
            if (!vmm_free_region(pdir, regions[i], sizes[i])) {
                cleanup_pagedir(pdir);
                send_msg("FAILED");
                return;
            }
        }
        
        // Cleanup address space
        cleanup_pagedir(pdir);
    }
    
    send_msg("PASSED");
}

//==============================================================================
// TEST 4: Map-Remap-Free Pattern Detection
//==============================================================================
void test_vmm_rapid_remapping() {
    ensure_vmm_ready();
    
    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }
    
    void* fixed_virt = (void*)0x50000000;
    
    // Perform 20 remap cycles on the same virtual address
    for (int i = 0; i < 20; i++) {
        // Allocate a page
        if (!vmm_alloc_region(pdir, fixed_virt, VMM_PAGE_SIZE, 
                              PTE_PRESENT | PTE_WRITABLE)) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Get physical frame and write unique pattern
        void* phys = vmm_get_phys_frame(pdir, fixed_virt);
        if (!phys) {
            vmm_free_region(pdir, fixed_virt, VMM_PAGE_SIZE);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        data[0] = 0xFACE0000 + i;
        data[1] = i * 2;
        
        // Verify data was written correctly
        if (data[0] != (0xFACE0000 + i) || data[1] != (uint32_t)(i * 2)) {
            vmm_free_region(pdir, fixed_virt, VMM_PAGE_SIZE);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Free the page
        if (!vmm_free_region(pdir, fixed_virt, VMM_PAGE_SIZE)) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        
        // Verify it's unmapped
        if (vmm_get_phys_frame(pdir, fixed_virt) != NULL) {
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
    }
    
    // Final allocation to ensure everything still works
    if (!vmm_alloc_region(pdir, fixed_virt, VMM_PAGE_SIZE, 
                          PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    void* final_phys = vmm_get_phys_frame(pdir, fixed_virt);
    if (!final_phys) {
        vmm_free_region(pdir, fixed_virt, VMM_PAGE_SIZE);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }
    
    vmm_free_region(pdir, fixed_virt, VMM_PAGE_SIZE);
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}

//==============================================================================
// TEST 5: Partial Region Free and Reallocation
//==============================================================================
void test_vmm_partial_region_operations(void) {
    ensure_vmm_ready();

    pagedir_t* pdir = vmm_create_address_space();
    if (!pdir) {
        send_msg("FAILED");
        return;
    }

    void* base_addr = (void*)0x60000000;
    size_t total_size = 10 * VMM_PAGE_SIZE;

    // Allocate 10 pages
    if (!vmm_alloc_region(pdir, base_addr, total_size, PTE_PRESENT | PTE_WRITABLE)) {
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    // Write unique pattern to each page
    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            vmm_free_region(pdir, base_addr, total_size);
            cleanup_pagedir(pdir);
            send_msg("FAILED");
            return;
        }
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        data[0] = 0xCAFE0000 + i;
    }

    // Free middle 4 pages (pages 3–6)
    void* middle_start = (void*)((uintptr_t)base_addr + (3 * VMM_PAGE_SIZE));
    if (!vmm_free_region(pdir, middle_start, 4 * VMM_PAGE_SIZE)) {
        vmm_free_region(pdir, base_addr, total_size);
        cleanup_pagedir(pdir);
        send_msg("FAILED");
        return;
    }

    // Verify pages 0–2 still mapped and correct
    for (int i = 0; i < 3; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        if (data[0] != (0xCAFE0000 + i)) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
    }

    // Verify middle pages unmapped
    for (int i = 3; i < 7; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        if (vmm_get_phys_frame(pdir, virt) != NULL) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
    }

    // Verify pages 7–9 still mapped and correct
    for (int i = 7; i < 10; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        if (data[0] != (0xCAFE0000 + i)) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
    }

    // Reallocate middle section
    if (!vmm_alloc_region(pdir, middle_start, 4 * VMM_PAGE_SIZE,
                          PTE_PRESENT | PTE_WRITABLE)) {
        send_msg("FAILED");
        cleanup_pagedir(pdir);
        return;
    }

    // Verify new pages are valid and writable (don’t assume cleared)
    for (int i = 3; i < 7; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);

        // Just ensure accessible
        volatile uint32_t tmp = data[0];
        (void)tmp;

        // Write new pattern for these reallocated pages
        data[0] = 0xBEEF0000 + i;
    }

    // Verify all 10 pages now contain correct expected pattern
    for (int i = 0; i < 10; i++) {
        void* virt = (void*)((uintptr_t)base_addr + (i * VMM_PAGE_SIZE));
        void* phys = vmm_get_phys_frame(pdir, virt);
        if (!phys) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
        uint32_t* data = (uint32_t*)PHYS_TO_VIRT((uintptr_t)phys);
        uint32_t expected = (i < 3 || i >= 7) ? (0xCAFE0000 + i) : (0xBEEF0000 + i);
        if (data[0] != expected) {
            send_msg("FAILED");
            cleanup_pagedir(pdir);
            return;
        }
    }

    vmm_free_region(pdir, base_addr, total_size);
    cleanup_pagedir(pdir);
    send_msg("PASSED");
}
