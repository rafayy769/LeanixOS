#include <proc/process.h>
#include <proc/elf.h>
#include <proc/tss.h>
#include <driver/serial.h>
#include <mm/kheap.h>
#include <utils.h>
#include <stdio.h>

#include <proc/process.h>
#include <string.h>
#include <testmain.h>

#ifndef PROCESS_PRI_MIN
#define PROCESS_PRI_MIN 0
#define PROCESS_PRI_MAX 10
#define PROCESS_PRI_DEFAULT 5
#endif


//-----------------------------------------------------------------------------
// TEST 1: Basic Process Creation (Non-NULL Return)
//-----------------------------------------------------------------------------
void test_process_create_nonnull(void)
{
    process_t* proc = malloc(sizeof(process_t));
    ASSERT_NOT_NULL(proc, "malloc failed for process");

    process_create(proc, "test_proc", PROCESS_PRI_DEFAULT);

    // Process should exist after creation
    ASSERT_NOT_NULL(proc, "process_create returned null");

    send_msg("PASSED: test_process_create_nonnull");
    process_destroy(proc);
    free(proc);
}

//-----------------------------------------------------------------------------
// TEST 2: Process Creation with Different Priorities
//-----------------------------------------------------------------------------
void test_process_create_different_priorities(void)
{
    process_t* proc1 = malloc(sizeof(process_t));
    process_t* proc2 = malloc(sizeof(process_t));
    process_t* proc3 = malloc(sizeof(process_t));

    // Just verify these don't crash with different priorities
    process_create(proc1, "min_pri", PROCESS_PRI_MIN);
    ASSERT_NOT_NULL(proc1, "process with min priority failed");

    process_create(proc2, "max_pri", PROCESS_PRI_MAX);
    ASSERT_NOT_NULL(proc2, "process with max priority failed");

    process_create(proc3, "def_pri", PROCESS_PRI_DEFAULT);
    ASSERT_NOT_NULL(proc3, "process with default priority failed");

    send_msg("PASSED: test_process_create_different_priorities");
    
    process_destroy(proc1);
    process_destroy(proc2);
    process_destroy(proc3);
    free(proc1);
    free(proc2);
    free(proc3);
}

//-----------------------------------------------------------------------------
// TEST 3: Process Name Handling
//-----------------------------------------------------------------------------
void test_process_name_handling(void)
{
    process_t* proc1 = malloc(sizeof(process_t));
    process_t* proc2 = malloc(sizeof(process_t));
    
    // Create with normal name
    process_create(proc1, "normal", PROCESS_PRI_DEFAULT);
    ASSERT_NOT_NULL(proc1, "process with normal name failed");

    // Create with long name (should not crash)
    char long_name[64];
    memset(long_name, 'X', 63);
    long_name[63] = '\0';
    process_create(proc2, long_name, PROCESS_PRI_DEFAULT);
    ASSERT_NOT_NULL(proc2, "process with long name failed");

    send_msg("PASSED: test_process_name_handling");
    
    process_destroy(proc1);
    process_destroy(proc2);
    free(proc1);
    free(proc2);
}

//-----------------------------------------------------------------------------
// TEST 4: Get Main Thread Returns Non-NULL
//-----------------------------------------------------------------------------
void test_get_main_thread_nonnull(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "has_thread", PROCESS_PRI_DEFAULT);

    thread_t* main_thread = _get_main_thread(proc);
    ASSERT_NOT_NULL(main_thread, "main thread should exist after process creation");

    send_msg("PASSED: test_get_main_thread_nonnull");
    
    process_destroy(proc);
    free(proc);
}

//-----------------------------------------------------------------------------
// TEST 5: Thread Creation
//-----------------------------------------------------------------------------
static void dummy_thread_func(void* arg)
{
    // Simple function that does nothing
    while(1) {}
}

void test_thread_creation(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "thread_test", PROCESS_PRI_DEFAULT);

    // Create a new thread
    thread_t* thread = thread_create(proc, dummy_thread_func, NULL);
    ASSERT_NOT_NULL(thread, "thread_create failed");

    send_msg("PASSED: test_thread_creation");
    
    thread_destroy(thread);
    process_destroy(proc);
    free(proc);
}

//-----------------------------------------------------------------------------
// TEST 6: Multiple Thread Creation
//-----------------------------------------------------------------------------
void test_multiple_thread_creation(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "multi_thread", PROCESS_PRI_DEFAULT);

    // Create multiple threads - should all succeed
    thread_t* t1 = thread_create(proc, dummy_thread_func, NULL);
    thread_t* t2 = thread_create(proc, dummy_thread_func, NULL);
    thread_t* t3 = thread_create(proc, dummy_thread_func, NULL);

    ASSERT_NOT_NULL(t1, "thread 1 creation failed");
    ASSERT_NOT_NULL(t2, "thread 2 creation failed");
    ASSERT_NOT_NULL(t3, "thread 3 creation failed");

    // All threads should be different pointers
    ASSERT_TRUE(t1 != t2, "thread 1 and 2 have same address");
    ASSERT_TRUE(t1 != t3, "thread 1 and 3 have same address");
    ASSERT_TRUE(t2 != t3, "thread 2 and 3 have same address");

    send_msg("PASSED: test_multiple_thread_creation");
    
    thread_destroy(t1);
    thread_destroy(t2);
    thread_destroy(t3);
    process_destroy(proc);
    free(proc);
}

//-----------------------------------------------------------------------------
// TEST 7: Scheduler Get Current Thread
//-----------------------------------------------------------------------------
void test_scheduler_get_current_thread(void)
{
    // Should always have a current thread in a running system
    thread_t* current = get_current_thread();
    ASSERT_NOT_NULL(current, "no current thread");

    send_msg("PASSED: test_scheduler_get_current_thread");
}

//-----------------------------------------------------------------------------
// TEST 8: Scheduler Get Current Process
//-----------------------------------------------------------------------------
void test_scheduler_get_current_proc(void)
{
    // Should always have a current process in a running system
    process_t* current = get_current_proc();
    ASSERT_NOT_NULL(current, "no current process");

    send_msg("PASSED: test_scheduler_get_current_proc");
}

//-----------------------------------------------------------------------------
// TEST 9: Scheduler Post Thread
//-----------------------------------------------------------------------------
static volatile int post_thread_executed = 0;

static void post_thread_func(void* arg)
{
    post_thread_executed = 1;
    while(1) {}
}

void test_scheduler_post_thread(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "post_test", PROCESS_PRI_DEFAULT);

    post_thread_executed = 0;

    // Create a thread with a valid entry point
    thread_t* thread = thread_create(proc, post_thread_func, NULL);
    ASSERT_NOT_NULL(thread, "thread creation failed");

    // Post thread to scheduler
    cli();
    scheduler_post(thread);
    sti();

    // Wait for the thread to execute (with timeout)
    int timeout = 0x7FFFFFFF;
    while (post_thread_executed == 0 && timeout-- > 0) {
        // Busy wait, allowing scheduler to run
    }

    ASSERT_TRUE(post_thread_executed == 1, "posted thread never executed");
    thread->state = STATE_TERMINATED; // prevent further scheduling
    send_msg("PASSED: test_scheduler_post_thread");
    
    // Note: Don't destroy as thread is now running
}

//-----------------------------------------------------------------------------
// TEST 10: Multiple Process Creation (Unique Processes)
//-----------------------------------------------------------------------------
void test_multiple_process_creation(void)
{
    #define NUM_PROCS 5
    process_t* procs[NUM_PROCS];

    // Create multiple processes
    for (int i = 0; i < NUM_PROCS; i++) {
        procs[i] = malloc(sizeof(process_t));
        ASSERT_NOT_NULL(procs[i], "malloc failed");
        
        char name[16];
        name[0] = 'p';
        utoa(i, &name[1]);
        process_create(procs[i], name, PROCESS_PRI_DEFAULT);
        ASSERT_NOT_NULL(procs[i], "process creation failed");
    }

    // Verify all are different pointers
    for (int i = 0; i < NUM_PROCS; i++) {
        for (int j = i + 1; j < NUM_PROCS; j++) {
            ASSERT_TRUE(procs[i] != procs[j], "duplicate process pointers");
        }
    }

    send_msg("PASSED: test_multiple_process_creation");

    // Cleanup
    for (int i = 0; i < NUM_PROCS; i++) {
        process_destroy(procs[i]);
        free(procs[i]);
    }
    #undef NUM_PROCS
}

//-----------------------------------------------------------------------------
// TEST 11: Thread Destroy Doesn't Crash
//-----------------------------------------------------------------------------
void test_thread_destroy_safe(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "thread_destroy", PROCESS_PRI_DEFAULT);

    thread_t* thread = thread_create(proc, dummy_thread_func, NULL);
    ASSERT_NOT_NULL(thread, "thread creation failed");

    // Destroy should not crash
    int32_t result = thread_destroy(thread);
    // Just verify it returns something (0 or -1 typically)
    ASSERT_TRUE(result == 0 || result == -1, "unexpected destroy return");

    send_msg("PASSED: test_thread_destroy_safe");
    
    process_destroy(proc);
    free(proc);
}


//-----------------------------------------------------------------------------
// TEST 13: Process Exit Function
//-----------------------------------------------------------------------------
void test_process_exit_safe(void)
{
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "exit_test", PROCESS_PRI_DEFAULT);

    // process_exit should not crash
    process_exit(proc, 0);
    process_exit(proc, 1);
    process_exit(proc, -1);

    send_msg("PASSED: test_process_exit_safe");
    
    process_destroy(proc);
    free(proc);
}

//-----------------------------------------------------------------------------
// TEST 14: Concurrent Scheduler Operations
//-----------------------------------------------------------------------------
static volatile int concurrent_executed[2] = {0, 0};

static void concurrent_func1(void* arg)
{
	concurrent_executed[0] = 1;
	while(1) {}
}
static void concurrent_func2(void* arg)
{
	concurrent_executed[1] = 1;
	while(1) {}
}

void test_concurrent_scheduler_ops(void)
{
    process_t* proc1 = malloc(sizeof(process_t));
    
    process_create(proc1, "concurrent1", PROCESS_PRI_DEFAULT);

    concurrent_executed[0] = 0;
    concurrent_executed[1] = 0;

    // Create threads with valid entry points
    thread_t* t1 = thread_create(proc1, concurrent_func1, NULL);
    thread_t* t2 = thread_create(proc1, concurrent_func2, NULL);

    ASSERT_NOT_NULL(t1, "thread 1 creation failed");
    ASSERT_NOT_NULL(t2, "thread 2 creation failed");

    // Post both threads - should handle concurrent posts
    cli();
    scheduler_post(t1);
    scheduler_post(t2);
    sti();

    // Wait for both threads to execute
    int timeout = 0x7FFFFFFF;
    while ((concurrent_executed[0] == 0 || concurrent_executed[1] == 0) && timeout-- > 0) {
        // Busy wait, allowing scheduler to run both threads
    }
    t1->state = STATE_TERMINATED; // prevent further scheduling
    t2->state = STATE_TERMINATED;

    ASSERT_TRUE(concurrent_executed[1] == 1, "thread 2 never executed");
    ASSERT_TRUE(concurrent_executed[0] == 1, "thread 1 never executed");

    send_msg("PASSED: test_concurrent_scheduler_ops");
    
}

//-----------------------------------------------------------------------------
// TEST 15: Stress Test - Many Threads
//-----------------------------------------------------------------------------
static volatile int stress_executed_count = 0;

static void stress_thread_func()
{
    stress_executed_count++;
    while(1) {}
}

void test_many_threads_creation(void)
{
    #define THREAD_COUNT 20
    process_t* proc = malloc(sizeof(process_t));
    process_create(proc, "stress", PROCESS_PRI_DEFAULT);

    thread_t* threads[THREAD_COUNT];
    int created_count = 0;
    stress_executed_count = 0;

    // Try to create many threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i] = thread_create(proc, stress_thread_func, NULL);
        if (threads[i] != NULL) {
            created_count++;
        }
    }

    // Should be able to create at least some threads
    ASSERT_TRUE(created_count > 0, "could not create any threads");

    // Post all created threads to scheduler
    cli();
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (threads[i] != NULL) {
            scheduler_post(threads[i]);
        }
    }
    sti();

    // Wait for threads to start executing
    int timeout = 0x7FFFFFFF;
    while (stress_executed_count < created_count && timeout-- > 0) {
        // Busy wait
    }

    // At least some threads should have executed
    ASSERT_TRUE(stress_executed_count > 0, "no threads executed");

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (threads[i] != NULL) {
            threads[i]->state = STATE_TERMINATED; // prevent further scheduling
        }
    }

    char msg[64];
    strcpy(msg, "PASSED: test_many_threads_creation (created=");
    char num[16];
    utoa(created_count, num);
    strcat(msg, num);
    strcat(msg, ", executed=");
    utoa(stress_executed_count, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);

    #undef THREAD_COUNT
}

//-----------------------------------------------------------------------------
// TEST 16: Three threads with interleaved output
//-----------------------------------------------------------------------------

static void thread_test1 (void) {
	for (volatile int i = 0; i < 100; i++)
		serial_puts  ("a");

	while (1) {}
}

static void thread_test2 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("b");

	while (1) {}

}

static void thread_test3 (void) {
	for (volatile int i = 0; i < 1000; i++)
		serial_puts ("c");

	serial_puts("*");
	while (1) {}
	
}

void thread_test (void) {

	// create three processes with one thread each
	process_t* proc1 = malloc (sizeof(process_t));
	process_t* proc2 = malloc (sizeof(process_t));
	process_t* proc3 = malloc (sizeof(process_t));

	process_create (proc1, "proc1", PROCESS_PRI_DEFAULT);
	process_create (proc2, "proc2", PROCESS_PRI_DEFAULT);
	process_create (proc3, "proc3", PROCESS_PRI_DEFAULT);

	thread_t* thread1 = _get_main_thread (proc1);
	thread_t* thread2 = _get_main_thread (proc2);
	thread_t* thread3 = _get_main_thread (proc3);

	thread1->trap_frame->eip = (uint32_t) thread_test1;
	thread2->trap_frame->eip = (uint32_t) thread_test2;
	thread3->trap_frame->eip = (uint32_t) thread_test3;

	cli ();
	scheduler_post(thread1);
	scheduler_post(thread2);
	scheduler_post(thread3);
	sti ();

    // wait for threads to execute
    int timeout = 0x7FFFFF;
    while (timeout-- > 0) {
        // Busy wait
    }

    thread1->state = STATE_TERMINATED;
    thread2->state = STATE_TERMINATED;
    thread3->state = STATE_TERMINATED;
}


//-----------------------------------------------------------------------------
// Hidden test cases
//-----------------------------------------------------------------------------

// HIDDEN TESTS
// Scheduling Test Hard
static void thread_test4 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("d");

	while (1) {}

}

static void thread_test5 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("e");

	while (1) {}

}

static void thread_test6 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("f");

	while (1) {}

}

static void thread_test7 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("g");

	while (1) {}

}

static void thread_test8 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("h");

	while (1) {}

}

static void thread_test9 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("i");

	while (1) {}

}

static void thread_test10 (void) {
	for (volatile int i = 0; i < 200; i++)
		serial_puts ("j");

	while (1) {}

}

void test_scheduler_ordering(void) {

    process_t* proc1  = malloc(sizeof(process_t));
    process_t* proc2  = malloc(sizeof(process_t));
    process_t* proc3  = malloc(sizeof(process_t));
    process_t* proc4  = malloc(sizeof(process_t));
    process_t* proc5  = malloc(sizeof(process_t));
    process_t* proc6  = malloc(sizeof(process_t));
    process_t* proc7  = malloc(sizeof(process_t));
    process_t* proc8  = malloc(sizeof(process_t));
    process_t* proc9  = malloc(sizeof(process_t));
    process_t* proc10 = malloc(sizeof(process_t));

    process_create (proc1, "1", PROCESS_PRI_DEFAULT);
    process_create (proc2, "2", PROCESS_PRI_DEFAULT);
    process_create (proc3, "3", PROCESS_PRI_DEFAULT);
    process_create (proc4, "4", PROCESS_PRI_DEFAULT);
    process_create (proc5, "5", PROCESS_PRI_DEFAULT);
    process_create (proc6, "6", PROCESS_PRI_DEFAULT);
    process_create (proc7, "7", PROCESS_PRI_DEFAULT);
    process_create (proc8, "8", PROCESS_PRI_DEFAULT);
    process_create (proc9, "9", PROCESS_PRI_DEFAULT);
    process_create (proc10, "10", PROCESS_PRI_DEFAULT);

    thread_t* thread1  = _get_main_thread (proc1);
    thread_t* thread2  = _get_main_thread (proc2);
    thread_t* thread3  = _get_main_thread (proc3);
    thread_t* thread4  = _get_main_thread (proc4);
    thread_t* thread5  = _get_main_thread (proc5);
    thread_t* thread6  = _get_main_thread (proc6);
    thread_t* thread7  = _get_main_thread (proc7);
    thread_t* thread8  = _get_main_thread (proc8);
    thread_t* thread9  = _get_main_thread (proc9);
    thread_t* thread10 = _get_main_thread (proc10);
    
	thread1->trap_frame->eip  = (uint32_t) thread_test1;
	thread2->trap_frame->eip  = (uint32_t) thread_test2;
	thread3->trap_frame->eip  = (uint32_t) thread_test4;
	thread4->trap_frame->eip  = (uint32_t) thread_test5;
	thread5->trap_frame->eip  = (uint32_t) thread_test6;
	thread6->trap_frame->eip  = (uint32_t) thread_test7;
	thread7->trap_frame->eip  = (uint32_t) thread_test8;
	thread8->trap_frame->eip  = (uint32_t) thread_test9;
	thread9->trap_frame->eip  = (uint32_t) thread_test10;
	thread10->trap_frame->eip = (uint32_t) thread_test3;

    cli ();
	scheduler_post (thread1);
	scheduler_post (thread2);
    scheduler_post (thread3);
    scheduler_post (thread4);
    scheduler_post (thread5);
    scheduler_post (thread6);
    scheduler_post (thread7);
    scheduler_post (thread8);
    scheduler_post (thread9);
    scheduler_post (thread10);
	sti ();

    // wait for threads to execute
    int timeout = 0x7FFFFF;
    while (timeout-- > 0) {
        // Busy wait
    }

    thread1->state  = STATE_TERMINATED;
    thread2->state  = STATE_TERMINATED;
    thread3->state  = STATE_TERMINATED;
    thread4->state  = STATE_TERMINATED;
    thread5->state  = STATE_TERMINATED;
    thread6->state  = STATE_TERMINATED;
    thread7->state  = STATE_TERMINATED;
    thread8->state  = STATE_TERMINATED;
    thread9->state  = STATE_TERMINATED;
    thread10->state = STATE_TERMINATED;
}

/* ORDERING TEST TWO */

static void func1 (void) {
	serial_puts ("D");
    serial_puts ("r");
    serial_puts (".");
    serial_puts (" ");
	while (1) {}
}

static void func2 (void) {
	serial_puts ("N");
	while (1) {}
}

static void func3 (void) {
	serial_puts ("a");
	while (1) {}
}

static void func4 (void) {
	serial_puts ("v");
	while (1) {}
}

static void func5 (void) {
	serial_puts ("e");
    serial_puts ("e");
	while (1) {}
}

static void func6 (void) {
	serial_puts ("d");
    serial_puts (" ");
	while (1) {}
}

static void func7 (void) {
	serial_puts ("G");
	while (1) {}
}

static void func8 (void) {
	serial_puts ("O");
	while (1) {}
}

static void func9 (void) {
	serial_puts ("A");
	while (1) {}
}

static void func10 (void) {
	serial_puts ("T");
    serial_puts ("*");
	while (1) {}
}

void test_scheduler_ordering_two(void) {

    process_t* proc1  = malloc (sizeof(process_t));
    process_t* proc2  = malloc (sizeof(process_t));
    process_t* proc3  = malloc (sizeof(process_t));
    process_t* proc4  = malloc (sizeof(process_t));
    process_t* proc5  = malloc (sizeof(process_t));
    process_t* proc6  = malloc (sizeof(process_t));
    process_t* proc7  = malloc (sizeof(process_t));
    process_t* proc8  = malloc (sizeof(process_t));
    process_t* proc9  = malloc (sizeof(process_t));
    process_t* proc10 = malloc (sizeof(process_t));

    process_create (proc1,   "1", PROCESS_PRI_DEFAULT);
    process_create (proc2,   "2", PROCESS_PRI_DEFAULT);
    process_create (proc3,   "3", PROCESS_PRI_DEFAULT);
    process_create (proc4,   "4", PROCESS_PRI_DEFAULT);
    process_create (proc5,   "5", PROCESS_PRI_DEFAULT);
    process_create (proc6,   "6", PROCESS_PRI_DEFAULT);
    process_create (proc7,   "7", PROCESS_PRI_DEFAULT);
    process_create (proc8,   "8", PROCESS_PRI_DEFAULT);
    process_create (proc9,   "9", PROCESS_PRI_DEFAULT);
    process_create (proc10, "10", PROCESS_PRI_DEFAULT);

    thread_t* thread1  = _get_main_thread (proc1);
    thread_t* thread2  = _get_main_thread (proc2);
    thread_t* thread3  = _get_main_thread (proc3);
    thread_t* thread4  = _get_main_thread (proc4);
    thread_t* thread5  = _get_main_thread (proc5);
    thread_t* thread6  = _get_main_thread (proc6);
    thread_t* thread7  = _get_main_thread (proc7);
    thread_t* thread8  = _get_main_thread (proc8);
    thread_t* thread9  = _get_main_thread (proc9);
    thread_t* thread10 = _get_main_thread (proc10);
    
	thread1->trap_frame->eip  = (uint32_t) func1;
	thread2->trap_frame->eip  = (uint32_t) func2;
	thread3->trap_frame->eip  = (uint32_t) func3;
	thread4->trap_frame->eip  = (uint32_t) func4;
	thread5->trap_frame->eip  = (uint32_t) func5;
	thread6->trap_frame->eip  = (uint32_t) func6;
	thread7->trap_frame->eip  = (uint32_t) func7;
	thread8->trap_frame->eip  = (uint32_t) func8;
	thread9->trap_frame->eip  = (uint32_t) func9;
	thread10->trap_frame->eip = (uint32_t) func10;

    cli ();
	scheduler_post(thread1);
	scheduler_post(thread2);
    scheduler_post(thread3);
    scheduler_post(thread4);
    scheduler_post(thread5);
    scheduler_post(thread6);
    scheduler_post(thread7);
    scheduler_post(thread8);
    scheduler_post(thread9);
    scheduler_post(thread10);
	sti ();

    // wait for threads to execute
    int timeout = 0xFFFFF;
    while (timeout-- > 0) {
        // Busy wait
    }

    thread1->state  = STATE_TERMINATED;
    thread2->state  = STATE_TERMINATED;
    thread3->state  = STATE_TERMINATED;
    thread4->state  = STATE_TERMINATED;
    thread5->state  = STATE_TERMINATED;
    thread6->state  = STATE_TERMINATED;
    thread7->state  = STATE_TERMINATED;
    thread8->state  = STATE_TERMINATED;
    thread9->state  = STATE_TERMINATED;
    thread10->state = STATE_TERMINATED;

}

/* ORDERING TEST THREE */

static void func11 (void) {
	serial_puts ("P");
	while (1) {}
}

static void func22 (void) {
	serial_puts ("A");
	while (1) {}
}

static void func33 (void) {
	serial_puts ("3");
    serial_puts (" ");
	while (1) {}
}

static void func44 (void) {
	serial_puts ("G");
	while (1) {}
}

static void func55 (void) {
	serial_puts ("O");
	while (1) {}
}

static void func66 (void) {
	serial_puts ("A");
	while (1) {}
}

static void func77 (void) {
	serial_puts ("T");
	while (1) {}
}

static void func88 (void) {
	serial_puts ("E");
	while (1) {}
}

static void func99 (void) {
	serial_puts ("D");
	while (1) {}
}

static void func1010 (void) {
    serial_puts ("*");
	while (1) {}
}

void test_scheduler_ordering_three(void) {

    process_t* proc1 = malloc(sizeof(process_t));
    process_t* proc2 = malloc(sizeof(process_t));
    process_t* proc3 = malloc(sizeof(process_t));

    process_t* proc4 = malloc(sizeof(process_t));
    process_t* proc5 = malloc(sizeof(process_t));
    process_t* proc6 = malloc(sizeof(process_t));
    process_t* proc7 = malloc(sizeof(process_t));
    process_t* proc8 = malloc(sizeof(process_t));
    process_t* proc9 = malloc(sizeof(process_t));
    process_t* proc10 = malloc(sizeof(process_t));

    process_create(proc1, "1", PROCESS_PRI_DEFAULT);
    process_create(proc2, "2", PROCESS_PRI_DEFAULT);
    process_create(proc3, "3", PROCESS_PRI_DEFAULT);

    process_create(proc4, "4", PROCESS_PRI_DEFAULT);
    process_create(proc5, "5", PROCESS_PRI_DEFAULT);
    process_create(proc6, "6", PROCESS_PRI_DEFAULT);
    process_create(proc7, "7", PROCESS_PRI_DEFAULT);
    process_create(proc8, "8", PROCESS_PRI_DEFAULT);
    process_create(proc9, "9", PROCESS_PRI_DEFAULT);
    process_create(proc10, "10", PROCESS_PRI_DEFAULT);

    thread_t* thread1 = _get_main_thread (proc1);
	thread_t* thread2 = _get_main_thread (proc2);
    thread_t* thread3 = _get_main_thread (proc3);

    thread_t* thread4 = _get_main_thread (proc4);
    thread_t* thread5 = _get_main_thread (proc5);
    thread_t* thread6 = _get_main_thread (proc6);
    thread_t* thread7 = _get_main_thread (proc7);
    thread_t* thread8 = _get_main_thread (proc8);
    thread_t* thread9 = _get_main_thread (proc9);
    thread_t* thread10 = _get_main_thread (proc10);
    
    thread1->trap_frame->eip = (uint32_t) func11;
    thread2->trap_frame->eip = (uint32_t) func22;
    thread3->trap_frame->eip = (uint32_t) func33;
    thread4->trap_frame->eip = (uint32_t) func44;
    thread5->trap_frame->eip = (uint32_t) func55;
    thread6->trap_frame->eip = (uint32_t) func66;
    thread7->trap_frame->eip = (uint32_t) func77;
    thread8->trap_frame->eip = (uint32_t) func88;
    thread9->trap_frame->eip = (uint32_t) func99;
	thread10->trap_frame->eip = (uint32_t) func1010; 

    cli ();
	scheduler_post(thread1);
	scheduler_post(thread2);
    scheduler_post(thread3);
	sti ();

    // Busy wait to allow first three threads to finish
    for (volatile int i = 0; i < 0x3FFFFFF; i++) {
        /* busy wait */
    }

    thread1->state = STATE_TERMINATED;
    thread2->state = STATE_TERMINATED;
    thread3->state = STATE_TERMINATED;

    // Destroy proc1, proc2, proc3 to free up resources
    process_destroy(proc1);
    process_destroy(proc2);
    process_destroy(proc3);
    free(proc1);
    free(proc2);    
    free(proc3);

    cli ();
    scheduler_post(thread4);
    scheduler_post(thread5);
    scheduler_post(thread6);
    scheduler_post(thread7);
    scheduler_post(thread8);
    scheduler_post(thread9);
    scheduler_post(thread10);
    sti ();

    // Busy wait to allow last seven threads to finish
    for (volatile int i = 0; i < 0x3FFFFFF; i++) {
        /* busy wait */
    }

    thread4->state  = STATE_TERMINATED;
    thread5->state  = STATE_TERMINATED;
    thread6->state  = STATE_TERMINATED;
    thread7->state  = STATE_TERMINATED;
    thread8->state  = STATE_TERMINATED;
    thread9->state  = STATE_TERMINATED;
    thread10->state = STATE_TERMINATED;


    // process_destroy and free remaining processes
    process_destroy(proc4);
    process_destroy(proc5);
    process_destroy(proc6); 
    process_destroy(proc7);
    process_destroy(proc8);
    process_destroy(proc9);
    process_destroy(proc10);
    free(proc4);
    free(proc5);
    free(proc6);
    free(proc7);
    free(proc8);
    free(proc9);
    free(proc10);
}