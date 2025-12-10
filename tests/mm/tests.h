#ifndef _MM_TESTS_H
#define _MM_TESTS_H

// ----------------- KMM tests -----------------
extern void test_kmm_init_total(void);
extern void test_kmm_reserved_regions(void);
extern void test_kmm_alloc_all(void);
extern void test_kmm_alloc_alignment(void);
extern void test_kmm_reuse_freed(void);
extern void test_kmm_double_free(void);
extern void test_kmm_free_invalid(void);
extern void test_kmm_consistency(void);
extern void test_kmm_pattern_alloc_free(void);
extern void test_kmm_oom(void);
// -- hidden
extern void test_kmm_frame0_always_reserved_hidden(void);
extern void test_kmm_fuzz_hidden(void);

// ----------------- KHEAP (buddy allocator) tests -----------------
extern void test_kheap_init(void);
extern void test_kheap_alloc_small(void);
extern void test_kheap_alloc_exact(void);
extern void test_kheap_split(void);
extern void test_kheap_free_reuse(void);
extern void test_kheap_coalesce(void);
extern void test_kheap_double_free(void);
extern void test_kheap_invalid_free(void);
extern void test_kheap_realloc_shrink(void);
extern void test_kheap_realloc_expand(void);
extern void test_kheap_realloc_null(void);
extern void test_kheap_realloc_zero(void);
extern void test_kheap_oom(void);
extern void test_kheap_stress_pattern(void);
extern void test_kheap_fragmentation_coalescing(void);
extern void test_kheap_alignment_check(void);
extern void test_kheap_random_stress(void);
extern void test_kheap_realloc_integrity(void);
extern void test_kheap_buddy_multilevel(void);
extern void test_kheap_buddy_symmetry(void);

// ----------------- VMM (virtual memory manager) tests -----------------
extern void test_vmm_init(void); // test 8
extern void test_vmm_get_kerneldir(void); // 1
extern void test_vmm_get_current_pagedir(void); //2
extern void test_vmm_create_address_space(void); // 3
extern void test_vmm_switch_pagedir(void); // 4
extern void test_vmm_create_pt(void); // 5
extern void test_vmm_map_page_basic(void); // 6
extern void test_vmm_page_alloc(void); // 9
extern void test_vmm_page_free(void); // 10
extern void test_vmm_alloc_region(); // 11
extern void test_vmm_free_region();  // 12 
extern void test_vmm_get_phys_frame(void); // 7 
extern void test_vmm_double_mapping(void); // 13 
extern void test_vmm_clone_pagetable(void); // 14
extern void test_vmm_clone_pagedir(void); // 15
extern void test_vmm_memory_reuse_cycle(void);
extern void test_vmm_page_table_cleanup(void);
extern void test_vmm_rapid_remapping(void);
extern void test_vmm_partial_region_operations(void);
extern void test_vmm_multiple_address_spaces_stress(void); 

#endif // _MM_TESTS_H