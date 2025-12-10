#include <driver/serial.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <testmain.h>
#include <init/tests.h>
#include <mm/tests.h>
#include <proc/tests.h>
#include <fs/tests.h>

/* Minimal unsigned int → string converter */
void utoa(unsigned val, char *buf) {
    char tmp[16];
    int i = 0, j = 0;

    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (val > 0 && i < (int)sizeof(tmp)) {
        tmp[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

/* All commands received from the orchestrator are read in this buffer */

#define CMD_BUF_SIZE	128
static  char 	cmd_buf [CMD_BUF_SIZE];

/* List of all the test name to test function mappings. All commands are sent 
	as a single word, and the corresponding test is ran in the kernel. Support
	for params can be added later by using simple strtok calls. */

static struct test_case test_cases[] = {

	// ---- VGA tests ----
	{ "vga_entry", 				test_vga_entry },
	{ "vga_cursor", 			test_vga_cursor },
	{ "vga_entry_overwrite", 	test_vga_entry_overwrite },
	{ "vga_color", 				test_vga_entry_colors },
	{ "vga_entry_boundaries", 	test_vga_entry_boundaries },
	{ "vga_entry_hidden", 		test_vga_entry_hidden },
	{ "vga_cursor_hidden", 		test_vga_cursor_hidden },

	// ---- INTERRUPT tests ----
	{ "intr_reg", 				test_intr_reg },
	{ "intr_unreg", 			test_intr_unreg },
	{ "intr_unreg", 			test_intr_unreg },
	{ "intr_multi", 			test_intr_multi },

	// ---- KEYBOARD tests ----
	{ "kbd_basic", 				test_kbd_basic },
	{ "kbd_multi", 				test_kbd_multi },
	{ "kbd_capslock", 			test_kbd_capslock },
	{ "kbd_shift", 				test_kbd_shift },

	// ---- TTY tests ----
	{ "terminal_getc",          test_terminal_getc },
	{ "terminal_read",          test_terminal_read },
	{ "terminal_cursor",        test_terminal_cursor },
	{ "terminal_clear",         test_terminal_clear },
	{ "terminal_putc",          test_terminal_putc },
	{ "terminal_write",         test_terminal_write },
	{ "terminal_column",        test_terminal_column },
	{ "terminal_scroll",        test_terminal_scroll},
	{ "terminal_colour",        test_terminal_colour},
	{ "terminal_text_color",    test_terminal_text_color},
	{ "terminal_bg_color",      test_terminal_bg_color},
	{ "terminal_echo",          test_terminal_echo},

	// ---- SYSCALL TESTS ----
	{ "syscall_register",       test_syscall_register },
	{ "syscall_read",       	test_syscall_read },
	{ "syscall_write",       	test_syscall_write },
	{ "syscall_read_enforced",  test_syscall_read_enforced},
	{ "syscall_write_enforced", test_syscall_write_enforced},

	// ---- SHELL
	{ "shell_echo",				test_shell_echo },
	{ "shell_repeat",			test_shell_repeat_n },
	{ "shell_clear",			test_shell_clear },
	{ "shell_colour",			test_shell_text_colour },
	{ "shell_bgcolour",			test_shell_bg_colour },

    // ---- KHEAP tests ----
    { "kheap_init",           			test_kheap_init },
    { "kheap_alloc_small",    			test_kheap_alloc_small },
    { "kheap_alloc_exact",    			test_kheap_alloc_exact },
    { "kheap_split",          			test_kheap_split },
    { "kheap_free_reuse",     			test_kheap_free_reuse },
    { "kheap_coalesce",       			test_kheap_coalesce },
    { "kheap_double_free",    			test_kheap_double_free },
    { "kheap_invalid_free",   			test_kheap_invalid_free },
    { "kheap_realloc_shrink", 			test_kheap_realloc_shrink },
    { "kheap_realloc_expand", 			test_kheap_realloc_expand },
    { "kheap_realloc_null",   			test_kheap_realloc_null },
    { "kheap_realloc_zero",   			test_kheap_realloc_zero },
    { "kheap_oom",            			test_kheap_oom },
    { "kheap_stress_pattern", 			test_kheap_stress_pattern },
	{ "kheap_fragmentation_coalescing", test_kheap_fragmentation_coalescing },
	{ "kheap_alignment_check",          test_kheap_alignment_check },
	{ "kheap_random_stress",            test_kheap_random_stress },
	{ "kheap_realloc_integrity",        test_kheap_realloc_integrity },
	{ "kheap_buddy_multilevel",         test_kheap_buddy_multilevel },
	{ "kheap_buddy_symmetry",          	test_kheap_buddy_symmetry },

    // ---- KMM tests ----
    { "kmm_init_total",       	test_kmm_init_total },
    { "kmm_reserved",         	test_kmm_reserved_regions },
    { "kmm_alloc_all",        	test_kmm_alloc_all },
    { "kmm_alloc_align",      	test_kmm_alloc_alignment },
    { "kmm_reuse",            	test_kmm_reuse_freed },
    { "kmm_double_free",      	test_kmm_double_free },
    { "kmm_free_invalid",     	test_kmm_free_invalid },
    { "kmm_consistency",      	test_kmm_consistency },
    { "kmm_pattern",          	test_kmm_pattern_alloc_free },
    { "kmm_oom",              	test_kmm_oom },
	{ "kmm_frame0",				test_kmm_frame0_always_reserved_hidden},
	{ "kmm_fuzz_hidden",		test_kmm_fuzz_hidden},

    // ---- VMM tests ----
	{ "vmm_init",             					test_vmm_init },
    { "vmm_get_kerneldir",    					test_vmm_get_kerneldir },
    { "vmm_get_currentdir",   					test_vmm_get_current_pagedir },
    { "vmm_create_space",     					test_vmm_create_address_space },
    { "vmm_switch_dir",       					test_vmm_switch_pagedir },
    { "vmm_create_pt",        					test_vmm_create_pt },
    { "vmm_map_basic",        					test_vmm_map_page_basic },
    { "vmm_page_alloc",       					test_vmm_page_alloc },
    { "vmm_page_free",        					test_vmm_page_free },
	{ "vmm_alloc_region",						test_vmm_alloc_region },
	{ "vmm_free_region",						test_vmm_free_region },
    { "vmm_get_phys",         					test_vmm_get_phys_frame },
    { "vmm_double_map",       					test_vmm_double_mapping },
	{ "vmm_clone_pagetable",					test_vmm_clone_pagetable },
    { "vmm_clone_dir",        					test_vmm_clone_pagedir },
	{ "vmm_memory_reuse_cycle",					test_vmm_memory_reuse_cycle },
	{ "vmm_page_table_cleanup",					test_vmm_page_table_cleanup },
	{ "vmm_rapid_remapping",					test_vmm_rapid_remapping },
	{ "vmm_partial_region_operations",			test_vmm_partial_region_operations },
	{ "vmm_multiple_address_spaces_stress",     test_vmm_multiple_address_spaces_stress },

	// -- Timer tests 

	{ "test_tick_count_incrementing",		test_tick_count_incrementing },
	{ "test_sleep_duration",				test_sleep_duration },
	{ "test_multiple_sleeps",				test_multiple_sleeps },
	{ "test_timer_sleep_zero",				test_timer_sleep_zero },
	{ "test_timer_reinit",					test_timer_reinit },

	// -- TSS tests --

	{ "test_tss_global_access",				test_tss_global_access },
	{ "test_tss_esp0_update",				test_tss_esp0_update },
	{ "test_tss_layout_and_init",			test_tss_layout_and_init },

	// -- ELF Tests --

	{ "test_elf_check_valid_header",		test_elf_check_valid_header },
	{ "test_elf_load_program",				test_elf_load_program },
	{ "test_elf_load_nonexistent",			test_elf_load_nonexistent },
	{ "test_elf_load_null_args",			test_elf_load_null_args },
	{ "test_elf_check_header_content",		test_elf_check_header_content },
	{ "test_elf_bss_zeroing",				test_elf_bss_zeroing },

	// -- Process and threads tests --

	{ "test_thread",								thread_test },
	{ "test_process_create_nonnull",			  	test_process_create_nonnull },
	{ "test_process_create_different_priorities",	test_process_create_different_priorities },
	{ "test_process_name_handling",					test_process_name_handling },
	{ "test_get_main_thread_nonnull",				test_get_main_thread_nonnull },
	{ "test_scheduler_get_current_thread",			test_scheduler_get_current_thread },
	{ "test_scheduler_get_current_proc",			test_scheduler_get_current_proc },
	{ "test_thread_creation",						test_thread_creation },
	{ "test_multiple_thread_creation",				test_multiple_thread_creation },
	{ "test_scheduler_post_thread",					test_scheduler_post_thread },
	{ "test_multiple_process_creation",				test_multiple_process_creation },
	{ "test_thread_destroy_safe",					test_thread_destroy_safe },
	{ "test_process_exit_safe",						test_process_exit_safe },
	{ "test_concurrent_scheduler_ops",				test_concurrent_scheduler_ops },
	{ "test_many_threads_creation",					test_many_threads_creation },
	{ "test_scheduler_ordering",					test_scheduler_ordering },
	{ "test_scheduler_ordering_two",				test_scheduler_ordering_two },
	{ "test_scheduler_ordering_three",				test_scheduler_ordering_three },

	// -- HFS tests
	{ "test_01_format_mount", 					test_01_format_mount },
	{ "test_02_single_directory", 				test_02_single_directory },
	{ "test_03_nested_directories", 			test_03_nested_directories },
	{ "test_04_single_file_create", 			test_04_single_file_create },
	{ "test_05_small_file_write_read", 			test_05_small_file_write_read },
	{ "test_06_multiple_files_same_dir", 		test_06_multiple_files_same_dir },
	{ "test_07_write_multiple_files", 			test_07_write_multiple_files },
	{ "test_08_files_in_multiple_dirs", 		test_08_files_in_multiple_dirs },
	{ "test_09_complex_tree_structure", 		test_09_complex_tree_structure },
	{ "test_10_medium_file_direct_pointers", 	test_10_medium_file_direct_pointers },
	{ "test_11_large_file_indirect_pointer", 	test_11_large_file_indirect_pointer },
	{ "test_12_very_large_file", 				test_12_very_large_file },
	{ "test_13_multiple_large_files", 			test_13_multiple_large_files },
	{ "test_14_file_overwrite", 				test_14_file_overwrite },
	{ "test_15_write_at_offset", 				test_15_write_at_offset },
	{ "test_16_partial_operations", 			test_16_partial_operations },
	{ "test_17_deep_path_lookup", 				test_17_deep_path_lookup },
	{ "test_18_very_deep_nesting", 				test_18_very_deep_nesting },
	{ "test_19_inode_allocation_stress", 		test_19_inode_allocation_stress },
	{ "test_20_block_allocation_stress", 		test_20_block_allocation_stress },
	{ "test_21_mixed_operations_stress", 		test_21_mixed_operations_stress },
	// Hidden tests
	{ "test_h01_sparse_file_random_offsets",	test_h01_sparse_file_random_offsets },
	{ "test_h02_interleaved_file_operations",	test_h02_interleaved_file_operations },
	{ "test_h03_maximum_file_size",				test_h03_maximum_file_size },
	{ "test_h04_directory_entry_stress",		test_h04_directory_entry_stress },
	{ "test_h05_complex_overwrite_expansion",	test_h05_complex_overwrite_expansion },
	{ "test_h06_deep_tree_with_files",			test_h06_deep_tree_with_files },
	{ "test_h07_fragmented_writes",				test_h07_fragmented_writes },
	{ "test_h08_concurrent_large_file_growth",	test_h08_concurrent_large_file_growth },
	{ "test_h09_cross_boundary_edge_cases",		test_h09_cross_boundary_edge_cases },
	{ "test_h10_comprehensive_stress_test",		test_h10_comprehensive_stress_test },
	

	{ NULL, NULL } // marks the end of the array

};

/* Responsible for test orchestration inside the kernel. */

void start_tests () {

	/* testing cycle is simple, read command, dispatch to function, repeat */
	while (1) {

		size_t cmd_size = read_command ();
		if (cmd_size == 0) {
			continue; // empty command, ignore
		}

		bool found = false;

		for (size_t i = 0; test_cases[i].name != NULL; i++) {

			if (strcmp (cmd_buf, test_cases[i].name) == 0) {

				found = true;
				test_cases[i].func (); // call the test function
				break;

			}

		}

		if (!found) {
			send_msg ("Unknown command");
		}

	}

}

/* Reads a command from the serial port into cmd_buf and returns
   the number of characters read. The command is terminated by a any of these:
   '\0', '\n', '\r'. */

size_t read_command () {

	size_t start = 0;
	memset (cmd_buf, 0, CMD_BUF_SIZE);

	while (1) {
		char c = serial_getc ();

		if (c == '\0' || c == '\n' || c == '\r') {

			cmd_buf [start] = '\0';
			break;
		
		} else {

			if (start < CMD_BUF_SIZE - 1) {
				cmd_buf [start++] = c;
			}
		
		}	
	}

	return start;
}

/* Send a message string back to the server. A "*" is used to mark the end
	of the message. */

void send_msg (const char *msg) {
	serial_puts (msg);
	serial_putc ('*'); // end of message marker
}