#ifndef _PROC_TESTS_H
#define _PROC_TESTS_H


/* Tests list */

// -- Timer tests

extern void test_tick_count_incrementing(void);
extern void test_sleep_duration(void);
extern void test_multiple_sleeps(void);

/* hidden */
extern void test_timer_sleep_zero(void);
extern void test_timer_reinit(void);

// -- TSS tests --

extern void test_tss_global_access(void);
extern void test_tss_esp0_update(void);

/* hidden */
extern void test_tss_layout_and_init(void);

// -- ELF Tests

extern void test_elf_check_valid_header(void);
extern void test_elf_load_program(void);
extern void test_elf_load_nonexistent(void);

/* hidden */
extern void test_elf_load_null_args(void);
extern void test_elf_check_header_content(void);
extern void test_elf_bss_zeroing(void);

// -- Process and threads tests --

extern void test_process_create_nonnull (void);
extern void test_process_create_different_priorities (void);
extern void test_process_name_handling(void);
extern void test_get_main_thread_nonnull(void);

extern void test_scheduler_init_idempotent(void);
extern void test_scheduler_get_current_thread(void);
extern void test_scheduler_get_current_proc(void);

extern void test_thread_creation(void);
extern void test_multiple_thread_creation(void);
extern void test_scheduler_post_thread(void);
extern void test_multiple_process_creation(void);
extern void test_thread_destroy_safe(void);
extern void test_process_exit_safe(void);
extern void test_concurrent_scheduler_ops(void);
extern void test_many_threads_creation(void);
extern void thread_test (void);

/* hidden */
extern void test_scheduler_ordering (void);
extern void test_scheduler_ordering_two (void);
extern void test_scheduler_ordering_three (void);
extern void test_process_destroy (void);

#endif