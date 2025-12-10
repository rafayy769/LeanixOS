#include <proc/elf.h>
#include <fs/vfs.h>
#include <mm/vmm.h>
#include <string.h>
#include <stddef.h>

#include <testmain.h>

// Helper macros for ELF offsets based on the standard specification (and Manual Table 1)
// These are absolute byte offsets into the struct.
#define OFF_IDENT     0x00  // Magic number start (16 bytes)
#define OFF_TYPE      0x10  // e_type (2 bytes)
#define OFF_MACHINE   0x12  // e_machine (2 bytes)
#define OFF_VERSION   0x14  // e_version (4 bytes)

// Expected values (Standard x86 ELF)
#define MAGIC_SIG     0x464C457F // .ELF
#define MACH_X86      0x03       // EM_386
#define TYPE_EXEC     0x02       // ET_EXEC

//-----------------------------------------------------------------------------
// TEST 1: ELF Header Validation - Valid File
//-----------------------------------------------------------------------------
void test_elf_check_valid_header(void)
{
    // Open a known-good ELF file

	char name[12];
	strncpy(name, "/fd0/HELLO", 12);
    file_t* file = vfs_open(name, 0);
    ASSERT_NOT_NULL(file, "could not open HELLO file");
    
    // Read ELF header
    elf_header_t header;
    int32_t bytes_read = vfs_read(file, &header, sizeof(elf_header_t));
    ASSERT_TRUE(bytes_read == sizeof(elf_header_t), "could not read ELF header");
    
    send_msg("PASSED: test_elf_check_valid_header");
}

//-----------------------------------------------------------------------------
// TEST 2: ELF Load - Load Complete Program
//-----------------------------------------------------------------------------
void test_elf_load_program(void)
{
    // Create a page directory for loading
    pagedir_t* test_dir = vmm_get_current_pagedir();
    ASSERT_NOT_NULL(test_dir, "could not create address space");
    
    void* entry_point = NULL;
    
    // Load the HELLO program
	char name[12];
	strncpy(name, "/fd0/HELLO", 12);
    int32_t result = elf_load(name, test_dir, &entry_point);
    ASSERT_EQ(result, 0, "elf_load failed for HELLO");
    ASSERT_NOT_NULL(entry_point, "entry point not set after load");
    
    // Entry point should be a valid user space address
    ASSERT_TRUE((uint32_t)entry_point >= 0x100000, "entry point incorrect");
    
    // free the region
    vmm_free_region(test_dir, (void*)0x100000, 0x4000);

    char msg[64];
    strcpy(msg, "PASSED: test_elf_load_program (entry @ 0x");
    char num[16];
    utoa((uint32_t)entry_point, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
    
}

//-----------------------------------------------------------------------------
// TEST 3: ELF Load - Nonexistent File
//-----------------------------------------------------------------------------
void test_elf_load_nonexistent(void)
{
    pagedir_t* dir = vmm_get_current_pagedir();
    ASSERT_NOT_NULL(dir, "could not create address space");
    
    void* entry = NULL;
    
    // Try to load a file that doesn't exist
    int32_t result = elf_load("/fd0/DOESNOTEXIST", dir, &entry);
    
    // Should fail (non-zero return)
    ASSERT_TRUE(result != 0, "elf_load succeeded for nonexistent file");
    
    send_msg("PASSED: test_elf_load_nonexistent");
}

//-----------------------------------------------------------------------------
// HIDDEN TEST 1: ELF Load - NULL Pagedir Handling
//-----------------------------------------------------------------------------
void test_elf_load_null_args(void)
{
    void *entry = NULL;
    // Passing NULL page directory should fail gracefully
    // Note: Manual implies loading into "provided virtual address space dir" [cite: 118]
    int32_t result = elf_load("/fd0/HELLO", NULL, &entry);

    ASSERT_TRUE(result != 0, "elf_load should fail with NULL pagedir");

    send_msg("PASSED: test_elf_load_null_args");
}

//-----------------------------------------------------------------------------
// HIDDEN TEST 2: ELF Header Content Validation
//-----------------------------------------------------------------------------
// Rationale: This test manually constructs a valid ELF header using raw memory 
// operations, then corrupts specific fields by byte offset to ensure the 
// student's validator checks them. This works regardless of struct member names.
void test_elf_check_header_content(void)
{
    // 1. Create a raw buffer large enough for the header
    uint8_t test_header[sizeof(elf_header_t)];
    memset(test_header, 0, sizeof(elf_header_t));

    // 2. Setup a COMPLETELY VALID header first
    
    *(uint32_t*)(test_header + OFF_IDENT) = MAGIC_SIG; // Magic: 0x7F 'E' 'L' 'F'
    test_header[OFF_IDENT + 4] = 1; // Class: 32-bit (offset 4) -> 1
    test_header[OFF_IDENT + 5] = 1; // Data: Little Endian (offset 5) -> 1
    test_header[OFF_IDENT + 6] = 1; // Version: Current (offset 6) -> 1
    *(uint16_t*)(test_header + OFF_TYPE) = TYPE_EXEC; // Type: Executable
    *(uint16_t*)(test_header + OFF_MACHINE) = MACH_X86; // Machine: x86
    *(uint32_t*)(test_header + OFF_VERSION) = 1; // Version: Current

    // Cast to elf_header_t pointer for validation call
    elf_header_t* hdr_ptr = (elf_header_t*)test_header;

    // --- Sub-test A: Verify the "Good" header passes first ---
    if (!elf_check_hdr(hdr_ptr)) {
        ASSERT_TRUE(false, "elf_check_hdr rejected a manually constructed VALID header.");
    }

    // --- Sub-test B: Corrupt Magic Number ---
    test_header[1] = 'X'; 
    if (elf_check_hdr(hdr_ptr)) {
        ASSERT_TRUE(false, "elf_check_hdr passed with invalid Magic Number (byte 1)");
    }
    // Restore
    test_header[1] = 'E';

    // --- Sub-test C: Corrupt Machine Architecture ---
    *(uint16_t*)(test_header + OFF_MACHINE) = 0x28;
    if (elf_check_hdr(hdr_ptr)) {
        ASSERT_TRUE(false, "elf_check_hdr passed with invalid Architecture (ARM)");
    }
    // Restore
    *(uint16_t*)(test_header + OFF_MACHINE) = MACH_X86;

    // --- Sub-test D: Corrupt File Type ---
    // The manual implies we generally only run Executables (or maybe Shared Objects).
    // If your manual strictly requires ET_EXEC, this test is valid.
    *(uint16_t*)(test_header + OFF_TYPE) = 1;
    if (elf_check_hdr(hdr_ptr)) {
        ASSERT_TRUE(false, "elf_check_hdr passed with non-executable type (ET_REL)");
    }
    // Restore
    *(uint16_t*)(test_header + OFF_TYPE) = TYPE_EXEC;

    send_msg("PASSED: test_elf_validation_robustness");
}

//-----------------------------------------------------------------------------
// HIDDEN TEST 3: ELF Segment Load - The .bss Zeroing Logic
//-----------------------------------------------------------------------------
// This is the most critical hidden test. It verifies that if mem_size > file_size,
// the loader correctly zeros out the extra memory.
void test_elf_bss_zeroing(void)
{
    // 1. Get current pagedir
    pagedir_t *dir = vmm_get_current_pagedir();
    ASSERT_NOT_NULL(dir, "Context failure: No page directory");

    // 2. Open ANY valid file just to get a file handle (e.g., HELLO)
    // // We won't actually execute it, just read from it.
    file_t *f = vfs_open("/fd0/HELLO", 0);
    ASSERT_NOT_NULL(f, "Could not open /fd0/HELLO for fixture");

    // create a 32 bytes array which will act as a program header
    uint32_t elf_phdr[8];
    memset(elf_phdr, 0, sizeof(elf_phdr));

    // create a fake program header that specifies a bss segment
    elf_phdr[0] = ELF_PT_LOAD;        // type
    elf_phdr[1] = 0;                  // offset
    elf_phdr[2] = 0x00500000;         // vaddr
    elf_phdr[3] = 0;                  // paddr
    elf_phdr[4] = 0;                  // file_size
    elf_phdr[5] = 4096;               // mem_size (larger than file_size)
    elf_phdr[6] = ELF_PF_R | ELF_PF_W; // flags
    elf_phdr[7] = 4096;               // align

    // 4. Call the student's segment loader directly
    int32_t res = elf_load_seg(f, dir, (elf_phdr_t*)elf_phdr);
    ASSERT_EQ(res, 0, "elf_load_seg failed on manual segment");


    uint8_t *bss_data = (uint8_t *)(0x00500000);
    bool is_zeroed = true;
    // check to ensure memset/bzero happened
    for (size_t i = 0; i < 4096; i++) {
        if (bss_data[i] != 0) {
            is_zeroed = false;
            break;
        }
    }

    ASSERT_TRUE(is_zeroed, "elf_load_seg did not zero-out the .bss section (mem_size > file_size)");

    // Cleanup (optional, depends on your VMM implementation)
    // vfs_close(f);
    vmm_free_region(dir, (void*)0x00500000, 0x1000);

    send_msg("PASSED: test_elf_bss_zeroing");
}