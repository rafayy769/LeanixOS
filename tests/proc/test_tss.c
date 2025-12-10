#include <proc/tss.h>
#include <testmain.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define TSS_SIZE        104
#define OFF_LINK        0x00 // Previous Task Link
#define OFF_ESP0        0x04 // Ring 0 Stack Pointer  <-- CRITICAL
#define OFF_SS0         0x08 // Ring 0 Stack Segment  <-- CRITICAL
#define OFF_ESP1        0x0C // Ring 1 Stack Pointer
#define OFF_SS1         0x10 // Ring 1 Stack Segment
#define OFF_ESP2        0x14 // Ring 2 Stack Pointer
#define OFF_SS2         0x18 // Ring 2 Stack Segment
#define OFF_CR3         0x1C // Page Directory Base
#define OFF_EIP         0x20 
#define OFF_EFLAGS      0x24
#define OFF_IOMAP       0x66 // I/O Map Base Address (16-bit)

//-----------------------------------------------------------------------------
// TEST 1: TSS Global Structure Access
//-----------------------------------------------------------------------------
void test_tss_global_access(void)
{
    // Get pointer to global TSS
    tss_t* tss = tss_get_global();
    
    ASSERT_NOT_NULL(tss, "global TSS is null");
    
    // Verify we can read from it (should not crash)
    // TSS is a valid memory location
    volatile uint32_t test_read = tss->esp0;
    (void)test_read;
    // Get it again to verify consistency
    tss_t* tss2 = tss_get_global();
    ASSERT_NOT_NULL(tss2, "second global TSS call returned null");
    
    // Should return the same pointer (global/singleton)
    ASSERT_TRUE(tss == tss2, "global TSS pointer inconsistent");
    
    char msg[64];
    strcpy(msg, "PASSED: test_tss_global_access (tss @ 0x");
    char num[16];
    utoa((uint32_t)tss, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 2: TSS ESP0 Update
//-----------------------------------------------------------------------------
void test_tss_esp0_update(void)
{
    // Get global TSS
    tss_t* tss = tss_get_global();
    ASSERT_NOT_NULL(tss, "global TSS is null");
    
    // Save original esp0 value
    uint32_t original_esp0 = tss->esp0;
    
    // Update esp0 to a new value
    uint32_t test_esp0_1 = 0xDEADBEEF;
    tss_update_esp0(test_esp0_1);
    
    // Verify the update
    ASSERT_EQ(tss->esp0, test_esp0_1, "first esp0 update failed");
    
    // Update again with different value
    uint32_t test_esp0_2 = 0xCAFEBABE;
    tss_update_esp0(test_esp0_2);
    
    // Verify second update
    ASSERT_EQ(tss->esp0, test_esp0_2, "second esp0 update failed");
    
    // Update back to a reasonable value
    uint32_t test_esp0_3 = 0xC0000000;
    tss_update_esp0(test_esp0_3);
    ASSERT_EQ(tss->esp0, test_esp0_3, "third esp0 update failed");
    
    char msg[128];
    strcpy(msg, "PASSED: test_tss_esp0_update (");
    char num[16];
    utoa(original_esp0, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_1, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_2, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_3, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// HIDDEN TEST: TSS Hardware Layout & Initialization Compliance
//-----------------------------------------------------------------------------
// Rationale: This test verifies that the TSS structure matches the specific 
// byte-layout required by the x86 CPU. It bypasses struct member names by 
// using raw memory offsets. It also checks if critical hardware values 
// (SS0) were initialized correctly in gdt.c.
void test_tss_layout_and_init(void)
{
    tss_t* tss = tss_get_global();
    ASSERT_NOT_NULL(tss, "tss_get_global returned NULL");

    uint8_t* raw_tss = (uint8_t*)tss;

    // 2. Check Structure Size
    // The hardware TSS is exactly 104 bytes.
    if (sizeof(tss_t) != TSS_SIZE) {
        ASSERT_EQ(sizeof(tss_t), TSS_SIZE, "TSS struct size incorrect (must be 104 bytes)");
    }

    // 3. Check SS0 (Stack Segment for Ring 0) -- Offset 0x08
    uint16_t ss0_val = *(uint16_t*)(raw_tss + OFF_SS0);
    ASSERT_EQ(ss0_val, 0x10, "TSS SS0 (offset 0x08) not initialized to 0x10");

    // 5. Verify Update Mechanism works on the Raw Memory
    // We update esp0 via the function, then read it back via RAW OFFSET 0x04.
    // This proves 'esp0' is actually at offset 0x04 in their struct.
    uint32_t old_esp0 = *(uint32_t*)(raw_tss + OFF_ESP0);
    uint32_t test_val = 0xBAADF00D;
    
    tss_update_esp0(test_val);
    
    uint32_t new_esp0 = *(uint32_t*)(raw_tss + OFF_ESP0);
    ASSERT_EQ(new_esp0, test_val, "tss_update_esp0 did not write to offset 0x04 (ESP0)");

    // Restore
    tss_update_esp0(old_esp0);

    send_msg("PASSED: test_tss_layout_and_init");
}