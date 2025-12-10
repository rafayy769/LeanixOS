#include <driver/timer.h>
#include <testmain.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// TEST 1: Tick Count Incrementing
//-----------------------------------------------------------------------------
void test_tick_count_incrementing(void)
{
    // Get initial tick count
    uint32_t tick1 = get_system_tick_count();
    
    // Busy wait for a bit to let timer tick
    for (volatile int i = 0; i < 9000000; i++) {}
    
    // Get tick count again
    uint32_t tick2 = get_system_tick_count();
    
    // Tick count should have increased
    ASSERT_TRUE(tick2 > tick1, "tick count did not increment");
    
    // Wait again
    for (volatile int i = 0; i < 9000000; i++) {}
    
    uint32_t tick3 = get_system_tick_count();
    
    // Should continue incrementing
    ASSERT_TRUE(tick3 > tick2, "tick count stopped incrementing");
    
    char msg[64];
    strcpy(msg, "PASSED: test_tick_count_incrementing (ticks: ");
    char num[16];
    utoa(tick1, num);
    strcat(msg, num);
    strcat(msg, " -> ");
    utoa(tick2, num);
    strcat(msg, num);
    strcat(msg, " -> ");
    utoa(tick3, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 2: Sleep Duration Accuracy
//-----------------------------------------------------------------------------
void test_sleep_duration(void)
{
    // Get tick count before sleep
    uint32_t tick_before = get_system_tick_count();
    
    // Sleep for 100ms
    sleep(100);
    
    // Get tick count after sleep
    uint32_t tick_after = get_system_tick_count();
    
    // Calculate elapsed ticks
    uint32_t elapsed_ticks = tick_after - tick_before;
    
    // Verify we slept (ticks should have advanced)
    ASSERT_TRUE(elapsed_ticks > 0, "no ticks elapsed during sleep");
    
    // For a 100ms sleep at typical frequencies (e.g., 100Hz = 10ms per tick)
    // we expect roughly 10 ticks (±some tolerance for scheduling)
    // This is a loose check - just verify sleep actually suspended execution
    ASSERT_TRUE(elapsed_ticks >= 5, "sleep duration too short");
    ASSERT_TRUE(elapsed_ticks <= 200, "sleep duration too long");
    
    char msg[64];
    strcpy(msg, "PASSED: test_sleep_duration (100ms = ");
    char num[16];
    utoa(elapsed_ticks, num);
    strcat(msg, num);
    strcat(msg, " ticks)");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 3: Multiple Sleeps
//-----------------------------------------------------------------------------
void test_multiple_sleeps(void)
{
    uint32_t tick_start = get_system_tick_count();
    
    // Sleep three times for 50ms each
    sleep(50);
    uint32_t tick1 = get_system_tick_count();
    
    sleep(50);
    uint32_t tick2 = get_system_tick_count();
    
    sleep(50);
    uint32_t tick3 = get_system_tick_count();
    
    // Verify each sleep advanced the tick count
    ASSERT_TRUE(tick1 > tick_start, "first sleep did not advance ticks");
    ASSERT_TRUE(tick2 > tick1, "second sleep did not advance ticks");
    ASSERT_TRUE(tick3 > tick2, "third sleep did not advance ticks");
    
    // Total elapsed should be roughly equivalent to 150ms
    uint32_t total_elapsed = tick3 - tick_start;
    ASSERT_TRUE(total_elapsed > 0, "no total elapsed time");
    
    char msg[128];
    strcpy(msg, "PASSED: test_multiple_sleeps (3x50ms = ");
    char num[16];
    utoa(total_elapsed, num);
    strcat(msg, num);
    strcat(msg, " ticks, intervals: ");
    utoa(tick1 - tick_start, num);
    strcat(msg, num);
    strcat(msg, ",");
    utoa(tick2 - tick1, num);
    strcat(msg, num);
    strcat(msg, ",");
    utoa(tick3 - tick2, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// HIDDEN TEST 1: Zero Sleep
//-----------------------------------------------------------------------------
// Rationale: Verifies that sleep(0) returns immediately without hanging
// or waiting for a generic "1 tick".
void test_timer_sleep_zero(void)
{
    uint32_t tick_start = get_system_tick_count();
    
    // Should return immediately
    sleep(0);
    
    uint32_t tick_end = get_system_tick_count();
    
    // In a fast emulator, this might be 0 ticks.
    // In a slow one, maybe 1 tick. 
    // But it definitely shouldn't be > 1 (assuming 1000Hz).
    uint32_t diff = tick_end - tick_start;
    
    ASSERT_TRUE(diff <= 1, "sleep(0) took too long");
    
    send_msg("PASSED: test_timer_sleep_zero");
}

//-----------------------------------------------------------------------------
// HIDDEN TEST 2: Re-initialization & Frequency Scaling
//-----------------------------------------------------------------------------
// Rationale: The student code stores '_timer_frequency'. This test calls init
// again with a DIFFERENT frequency (e.g., 100Hz) and checks if sleep(100)
// adapts correctly.
// 100ms at 1000Hz = 100 ticks.
// 100ms at 100Hz  = 10 ticks.
void test_timer_reinit(void)
{
    // 1. Re-initialize to 100Hz (slower ticks)
    // The student code resets _timer_tick_count to 0 here.
    init_system_timer(100);
    
    uint32_t start = get_system_tick_count();
    
    // 2. Sleep 100ms
    sleep(100);
    
    uint32_t end = get_system_tick_count();
    uint32_t diff = end - start;
    
    // 3. Verify tick count
    // At 100Hz, 100ms is exactly 10 ticks.
    // We allow a small margin for emulator jitter (8 to 12).
    // If the code was hardcoded to 1000Hz math, it would have waited 100 ticks.
    
    if (diff > 50) {
        // This implies the math didn't update to the new frequency
        ASSERT_TRUE(false, "sleep() logic did not adapt to new frequency (waited too many ticks)");
    }
    
    ASSERT_TRUE(diff >= 8 && diff <= 15, "sleep(100ms) at 100Hz duration incorrect");
    
    // 4. Re-initialize back to default (1000Hz) for safety of other tests
    init_system_timer(1000);
    
    send_msg("PASSED: test_timer_reinit");
}