#ifndef _FS_TESTS_H
#define _FS_TESTS_H

/* HFS Filesystem Tests - Organized by Test Levels */

/* LEVEL 1: Basic Filesystem Operations */
extern void test_01_format_mount(void);

/* LEVEL 2: Single Directory Operations */
extern void test_02_single_directory(void);
extern void test_03_nested_directories(void);

/* LEVEL 3: Single File Operations */
extern void test_04_single_file_create(void);
extern void test_05_small_file_write_read(void);

/* LEVEL 4: Multiple Files in Same Directory */
extern void test_06_multiple_files_same_dir(void);
extern void test_07_write_multiple_files(void);

/* LEVEL 5: Files and Directories in Different Locations */
extern void test_08_files_in_multiple_dirs(void);
extern void test_09_complex_tree_structure(void);

/* LEVEL 6: Medium-Sized Files (Multiple Blocks, Direct Pointers Only) */
extern void test_10_medium_file_direct_pointers(void);

/* LEVEL 7: Large Files (Requiring Indirect Pointers) */
extern void test_11_large_file_indirect_pointer(void);
extern void test_12_very_large_file(void);
extern void test_13_multiple_large_files(void);

/* LEVEL 8: File Modification Operations */
extern void test_14_file_overwrite(void);
extern void test_15_write_at_offset(void);
extern void test_16_partial_operations(void);

/* LEVEL 9: Path Lookup and Deep Nesting */
extern void test_17_deep_path_lookup(void);
extern void test_18_very_deep_nesting(void);

/* LEVEL 10: Stress Tests and Resource Allocation */
extern void test_19_inode_allocation_stress(void);
extern void test_20_block_allocation_stress(void);
extern void test_21_mixed_operations_stress(void);

/* Hidden tests */

extern void test_h01_sparse_file_random_offsets(void);
extern void test_h02_interleaved_file_operations(void);
extern void test_h03_maximum_file_size(void);
extern void test_h04_directory_entry_stress(void);
extern void test_h05_complex_overwrite_expansion(void);
extern void test_h06_deep_tree_with_files(void);
extern void test_h07_fragmented_writes(void);
extern void test_h08_concurrent_large_file_growth(void);
extern void test_h09_cross_boundary_edge_cases(void);
extern void test_h10_comprehensive_stress_test(void);

#endif // _FS_TESTS_H
