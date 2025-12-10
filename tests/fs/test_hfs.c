#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <fs/hfs.h>
#include <fs/vfs.h>
#include <driver/block.h>
#include <testmain.h>

#define TEST_DEVICE "hd1"

/* Helper: ensure HFS is formatted and mounted */
static vfs* ensure_hfs_mounted(void) {
    static vfs* fs = NULL;
    if (!fs) {
        hfs_format(TEST_DEVICE);
        vfs_mount(TEST_DEVICE, "/test", "hfs");
        fs = vfs_get_mounted("/test");
    }
    return fs;
}

/* ============================================================================
 * LEVEL 1: Basic Filesystem Operations
 * ========================================================================== */

/* Test 01: Format and Mount */
void test_01_format_mount(void) {
    int ret = hfs_format(TEST_DEVICE);
    ASSERT_EQ(ret, 0, "format failed");

    ret = vfs_mount(TEST_DEVICE, "/test", "hfs");
    ASSERT_EQ(ret, 0, "mount failed");

    vfs* fs = vfs_get_mounted("/test");
    ASSERT_TRUE(fs != NULL, "filesystem not mounted");

    send_msg("PASSED");
}

/* ============================================================================
 * LEVEL 2: Single Directory Operations
 * ========================================================================== */

/* Test 02: Create a single directory */
void test_02_single_directory(void) {
    ensure_hfs_mounted();

    int ret = vfs_mkdir("/test/dir1");
    ASSERT_EQ(ret, 0, "mkdir failed");

    send_msg("PASSED");
}

/* Test 03: Create nested directories */
void test_03_nested_directories(void) {
    ensure_hfs_mounted();

    ASSERT_EQ(vfs_mkdir("/test/parent"), 0, "mkdir parent failed");
    ASSERT_EQ(vfs_mkdir("/test/parent/child"), 0, "mkdir child failed");
    ASSERT_EQ(vfs_mkdir("/test/parent/child/grandchild"), 0, "mkdir grandchild failed");

    send_msg("PASSED");
}

/* ============================================================================
 * LEVEL 3: Single File Operations
 * ========================================================================== */

/* Test 04: Create a single file */
void test_04_single_file_create(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/files");
    int ret = vfs_create("/test/files/test.txt", 0);
    ASSERT_EQ(ret, 0, "create file failed");

    send_msg("PASSED");
}

/* Test 05: Write and read small data (< 1 block) */
void test_05_small_file_write_read(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/small");
    vfs_create("/test/small/tiny.txt", 0);

    file_t* file = vfs_open("/test/small/tiny.txt", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    const char* test_data = "Hello HFS!";
    int written = vfs_write(file, (void*)test_data, strlen(test_data));
    ASSERT_EQ(written, (int)strlen(test_data), "write failed");
    vfs_close(file);

    file = vfs_open("/test/small/tiny.txt", 0);
    ASSERT_TRUE(file != NULL, "reopen failed");

    char read_buf[64];
    memset(read_buf, 0, sizeof(read_buf));
    int read_bytes = vfs_read(file, read_buf, strlen(test_data));
    ASSERT_EQ(read_bytes, (int)strlen(test_data), "read failed");

    vfs_close(file);
    send_msg(read_buf);
}

/* ============================================================================
 * LEVEL 4: Multiple Files in Same Directory
 * ========================================================================== */

/* Test 06: Create multiple files in one directory */
void test_06_multiple_files_same_dir(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/multifile");

    char filename[64];
    int success_count = 0;
    for (int i = 0; i < 30; i++) {
        strcpy(filename, "/test/multifile/file");
        char num[8];
        utoa(i, num);
        strcat(filename, num);
        strcat(filename, ".txt");

        int ret = vfs_create(filename, 0);
        if (ret == 0) {
            success_count++;
        }
    }

    ASSERT_EQ(success_count, 30, "not all files created");

    char result[32];
    strcpy(result, "created:");
    char num[8];
    utoa(success_count, num);
    strcat(result, num);
    send_msg(result);
}

/* Test 07: Write data to multiple files in same directory */
void test_07_write_multiple_files(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/multiwrite");

    int files_written = 0;
    for (int i = 0; i < 10; i++) {
        char filename[64];
        strcpy(filename, "/test/multiwrite/data");
        char num[8];
        utoa(i, num);
        strcat(filename, num);
        strcat(filename, ".txt");

        vfs_create(filename, 0);
        file_t* file = vfs_open(filename, 0);

        if (file) {
            char data[64];
            strcpy(data, "File number ");
            strcat(data, num);

            int written = vfs_write(file, data, strlen(data));
            vfs_close(file);

            if (written == (int)strlen(data)) {
                files_written++;
            }
        }
    }

    ASSERT_EQ(files_written, 10, "not all files written");

    // Verify we can read them back
    file_t* file = vfs_open("/test/multiwrite/data5.txt", 0);
    ASSERT_TRUE(file != NULL, "failed to reopen file");

    char verify[64];
    memset(verify, 0, sizeof(verify));
    vfs_read(file, verify, 64);
    vfs_close(file);

    send_msg(verify);
}

/* ============================================================================
 * LEVEL 5: Files and Directories in Different Locations
 * ========================================================================== */

/* Test 08: Multiple directories with files in each */
void test_08_files_in_multiple_dirs(void) {
    ensure_hfs_mounted();

    // Create multiple directories
    const char* dirs[] = {"/test/dir_a", "/test/dir_b", "/test/dir_c", "/test/dir_d"};
    int num_dirs = 4;

    for (int d = 0; d < num_dirs; d++) {
        ASSERT_EQ(vfs_mkdir(dirs[d]), 0, "mkdir failed");

        // Create 5 files in each directory
        for (int f = 0; f < 5; f++) {
            char filepath[128];
            strcpy(filepath, dirs[d]);
            strcat(filepath, "/file");
            char num[8];
            utoa(f, num);
            strcat(filepath, num);
            strcat(filepath, ".dat");

            ASSERT_EQ(vfs_create(filepath, 0), 0, "create failed");
        }
    }

    send_msg("PASSED:4dirs_5files_each");
}

/* Test 09: Complex directory tree with files */
void test_09_complex_tree_structure(void) {
    ensure_hfs_mounted();

    // Create a complex tree structure
    vfs_mkdir("/test/root");
    vfs_mkdir("/test/root/docs");
    vfs_mkdir("/test/root/images");
    vfs_mkdir("/test/root/videos");
    vfs_mkdir("/test/root/docs/personal");
    vfs_mkdir("/test/root/docs/work");

    // Create files at various levels
    const char* files[] = {
        "/test/root/readme.txt",
        "/test/root/docs/notes.txt",
        "/test/root/docs/todo.txt",
        "/test/root/docs/personal/diary.txt",
        "/test/root/docs/work/project.txt",
        "/test/root/images/photo1.jpg",
        "/test/root/images/photo2.jpg"
    };

    int created = 0;
    for (int i = 0; i < 7; i++) {
        if (vfs_create(files[i], 0) == 0) {
            created++;
        }
    }

    ASSERT_EQ(created, 7, "not all files created in tree");

    char result[32];
    strcpy(result, "tree_files:");
    char num[8];
    utoa(created, num);
    strcat(result, num);
    send_msg(result);
}

/* ============================================================================
 * LEVEL 6: Medium-Sized Files (Multiple Blocks, Direct Pointers Only)
 * ========================================================================== */

/* Test 10: Write and read file using multiple direct pointers */
void test_10_medium_file_direct_pointers(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/medium");
    vfs_create("/test/medium/medium.dat", 0);

    file_t* file = vfs_open("/test/medium/medium.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    // Write 3KB of data (6 blocks, still within direct pointers)
    char write_buf[3072];
    for (int i = 0; i < 3072; i++) {
        write_buf[i] = 'A' + (i % 26);
    }

    int written = vfs_write(file, write_buf, 3072);
    ASSERT_EQ(written, 3072, "write failed");
    vfs_close(file);

    // Read back and verify
    file = vfs_open("/test/medium/medium.dat", 0);
    ASSERT_TRUE(file != NULL, "reopen failed");

    char read_buf[3072];
    memset(read_buf, 0, sizeof(read_buf));
    int read_bytes = vfs_read(file, read_buf, 3072);
    ASSERT_EQ(read_bytes, 3072, "read failed");

    bool match = true;
    for (int i = 0; i < 3072; i++) {
        if (read_buf[i] != write_buf[i]) {
            match = false;
            break;
        }
    }
    ASSERT_TRUE(match, "data mismatch");

    vfs_close(file);
    send_msg("PASSED:3KB");
}

/* ============================================================================
 * LEVEL 7: Large Files (Requiring Indirect Pointers)
 * ========================================================================== */

/* Test 11: Large file requiring indirect pointer block */
void test_11_large_file_indirect_pointer(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/large");
    vfs_create("/test/large/big.dat", 0);

    file_t* file = vfs_open("/test/large/big.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    // Write 8KB of data (16 blocks)
    // Direct pointers: 13 * 512 = 6656 bytes
    // This requires indirect pointer for remaining data
    char pattern[512];
    for (int i = 0; i < 512; i++) {
        pattern[i] = (char)((i + 42) & 0xFF);
    }

    int total_written = 0;
    for (int block = 0; block < 16; block++) {
        int written = vfs_write(file, pattern, 512);
        if (written < 0) break;
        total_written += written;
    }

    ASSERT_TRUE(total_written >= 8192, "insufficient write");
    vfs_close(file);

    // Read back and verify pattern
    file = vfs_open("/test/large/big.dat", 0);
    ASSERT_TRUE(file != NULL, "reopen failed");

    char read_buf[512];
    bool pattern_match = true;

    for (int block = 0; block < 16; block++) {
        memset(read_buf, 0, sizeof(read_buf));
        int read_bytes = vfs_read(file, read_buf, 512);

        if (read_bytes != 512) {
            pattern_match = false;
            break;
        }

        for (int i = 0; i < 512; i++) {
            if (read_buf[i] != pattern[i]) {
                pattern_match = false;
                break;
            }
        }

        if (!pattern_match) break;
    }

    ASSERT_TRUE(pattern_match, "pattern mismatch in large file");
    vfs_close(file);

    send_msg("PASSED:8KB_indirect");
}

/* Test 12: Very large file with extensive indirect pointer usage */
void test_12_very_large_file(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/verylarge");
    vfs_create("/test/verylarge/huge.dat", 0);

    file_t* file = vfs_open("/test/verylarge/huge.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    // Write 20KB of data (40 blocks)
    // This heavily utilizes indirect pointer block
    char block_data[512];
    for (int i = 0; i < 512; i++) {
        block_data[i] = (char)(i & 0xFF);
    }

    int blocks_written = 0;
    for (int block = 0; block < 40; block++) {
        // Vary the pattern per block for verification
        block_data[0] = (char)block;

        int written = vfs_write(file, block_data, 512);
        if (written == 512) {
            blocks_written++;
        } else {
            break;
        }
    }

    ASSERT_TRUE(blocks_written >= 30, "too few blocks written");
    vfs_close(file);

    // Verify we can read from different offsets
    file = vfs_open("/test/verylarge/huge.dat", 0);
    ASSERT_TRUE(file != NULL, "reopen failed");

    // Read from middle of file (block 20)
    file->f_offset = 20 * 512;
    char verify_buf[512];
    int read_bytes = vfs_read(file, verify_buf, 512);
    ASSERT_TRUE(read_bytes > 0, "read from offset failed");
    ASSERT_EQ(verify_buf[0], 20, "wrong block data");

    vfs_close(file);

    char result[64];
    strcpy(result, "PASSED:blocks_written:");
    char num[8];
    utoa(blocks_written, num);
    strcat(result, num);
    send_msg(result);
}

/* Test 13: Multiple large files */
void test_13_multiple_large_files(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/biglot");

    int large_files_created = 0;

    for (int f = 0; f < 5; f++) {
        char filename[64];
        strcpy(filename, "/test/biglot/large");
        char num[8];
        utoa(f, num);
        strcat(filename, num);
        strcat(filename, ".dat");

        vfs_create(filename, 0);
        file_t* file = vfs_open(filename, 0);

        if (file) {
            // Write 10KB to each file (requires indirect pointers)
            char data[512];
            memset(data, 'X' + f, sizeof(data));

            int blocks = 0;
            for (int b = 0; b < 20; b++) {
                if (vfs_write(file, data, 512) == 512) {
                    blocks++;
                }
            }

            vfs_close(file);

            if (blocks >= 15) {  // At least 7.5KB written
                large_files_created++;
            }
        }
    }

    ASSERT_TRUE(large_files_created >= 3, "too few large files created");

    char result[64];
    strcpy(result, "large_files:");
    char num[8];
    utoa(large_files_created, num);
    strcat(result, num);
    send_msg(result);
}

/* ============================================================================
 * LEVEL 8: File Modification Operations
 * ========================================================================== */

/* Test 14: File overwrite */
void test_14_file_overwrite(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/overwrite");
    vfs_create("/test/overwrite/file.txt", 0);

    // Write initial data
    file_t* file = vfs_open("/test/overwrite/file.txt", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    const char* data1 = "First write";
    vfs_write(file, (void*)data1, strlen(data1));
    vfs_close(file);

    // Overwrite with new data
    file = vfs_open("/test/overwrite/file.txt", 0);
    const char* data2 = "Second write is longer!";
    vfs_write(file, (void*)data2, strlen(data2));
    vfs_close(file);

    // Read back and verify
    file = vfs_open("/test/overwrite/file.txt", 0);
    char buf[64];
    memset(buf, 0, sizeof(buf));
    int read_bytes = vfs_read(file, buf, strlen(data2));
    ASSERT_EQ(read_bytes, (int)strlen(data2), "read failed");

    vfs_close(file);
    send_msg(buf);
}

/* Test 15: Write at offset */
void test_15_write_at_offset(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/offset");
    vfs_create("/test/offset/file.txt", 0);

    file_t* file = vfs_open("/test/offset/file.txt", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    // Write initial data
    const char* data1 = "AAAAAAAAAA";
    vfs_write(file, (void*)data1, 10);

    // Write at offset
    file->f_offset = 5;
    const char* data2 = "BBBBB";
    vfs_write(file, (void*)data2, 5);

    vfs_close(file);

    // Read back
    file = vfs_open("/test/offset/file.txt", 0);
    char buf[32];
    memset(buf, 0, sizeof(buf));
    vfs_read(file, buf, 10);
    vfs_close(file);

    send_msg(buf);
}

/* Test 16: Partial reads and writes */
void test_16_partial_operations(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/partial");
    vfs_create("/test/partial/data.bin", 0);

    file_t* file = vfs_open("/test/partial/data.bin", 0);
    ASSERT_TRUE(file != NULL, "open failed");

    // Write 1KB
    char write_data[1024];
    for (int i = 0; i < 1024; i++) {
        write_data[i] = (char)(i & 0xFF);
    }

    int written = vfs_write(file, write_data, 1024);
    ASSERT_EQ(written, 1024, "full write failed");
    vfs_close(file);

    // Read in chunks
    file = vfs_open("/test/partial/data.bin", 0);

    char chunk1[256];
    char chunk2[512];
    char chunk3[256];

    int r1 = vfs_read(file, chunk1, 256);
    int r2 = vfs_read(file, chunk2, 512);
    int r3 = vfs_read(file, chunk3, 256);

    ASSERT_EQ(r1, 256, "chunk1 read failed");
    ASSERT_EQ(r2, 512, "chunk2 read failed");
    ASSERT_EQ(r3, 256, "chunk3 read failed");

    // Verify data integrity
    bool valid = true;
    for (int i = 0; i < 256; i++) {
        if (chunk1[i] != write_data[i]) valid = false;
    }
    for (int i = 0; i < 512; i++) {
        if (chunk2[i] != write_data[256 + i]) valid = false;
    }
    for (int i = 0; i < 256; i++) {
        if (chunk3[i] != write_data[768 + i]) valid = false;
    }

    ASSERT_TRUE(valid, "partial read data mismatch");
    vfs_close(file);

    send_msg("PASSED:partial_ops");
}

/* ============================================================================
 * LEVEL 9: Path Lookup and Deep Nesting
 * ========================================================================== */

/* Test 17: Deep path lookup */
void test_17_deep_path_lookup(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/a");
    vfs_mkdir("/test/a/b");
    vfs_mkdir("/test/a/b/c");
    vfs_mkdir("/test/a/b/c/d");
    vfs_create("/test/a/b/c/d/deep.txt", 0);

    file_t* file = vfs_open("/test/a/b/c/d/deep.txt", 0);
    ASSERT_TRUE(file != NULL, "path lookup failed");

    vfs_close(file);
    send_msg("PASSED");
}

/* Test 18: Very deep directory nesting */
void test_18_very_deep_nesting(void) {
    ensure_hfs_mounted();

    char path[256] = "/test/deepnest";
    vfs_mkdir(path);

    int depth = 0;
    for (int i = 0; i < 10; i++) {
        strcat(path, "/level");
        char num[8];
        utoa(i, num);
        strcat(path, num);

        int ret = vfs_mkdir(path);
        if (ret == 0) {
            depth++;
        } else {
            break;
        }
    }

    ASSERT_TRUE(depth >= 7, "insufficient nesting depth");

    char result[32];
    strcpy(result, "depth:");
    char num[8];
    utoa(depth, num);
    strcat(result, num);
    send_msg(result);
}

/* ============================================================================
 * LEVEL 10: Stress Tests and Resource Allocation
 * ========================================================================== */

/* Test 19: Inode allocation stress test */
void test_19_inode_allocation_stress(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/inodes");

    int inodes_allocated = 0;

    // Create many directories to allocate inodes
    for (int i = 0; i < 50; i++) {
        char dirname[64];
        strcpy(dirname, "/test/inodes/dir");
        char num[8];
        utoa(i, num);
        strcat(dirname, num);

        int ret = vfs_mkdir(dirname);
        if (ret == 0) {
            inodes_allocated++;
        }
    }

    ASSERT_TRUE(inodes_allocated >= 30, "too few inodes allocated");

    char result[32];
    strcpy(result, "inodes:");
    char num[8];
    utoa(inodes_allocated, num);
    strcat(result, num);
    send_msg(result);
}

/* Test 20: Block allocation stress test */
void test_20_block_allocation_stress(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/blocks");

    int files_created = 0;

    // Create many files with data to stress block allocation
    for (int i = 0; i < 20; i++) {
        char filename[64];
        strcpy(filename, "/test/blocks/file");
        char num[8];
        utoa(i, num);
        strcat(filename, num);

        vfs_create(filename, 0);
        file_t* file = vfs_open(filename, 0);

        if (file) {
            char data[1024];
            memset(data, 'B', sizeof(data));
            int written = vfs_write(file, data, 1024);
            vfs_close(file);

            if (written >= 512) {  // At least one block
                files_created++;
            }
        }
    }

    ASSERT_TRUE(files_created >= 15, "block allocation failed");

    char result[64];
    strcpy(result, "files_with_blocks:");
    char num[8];
    utoa(files_created, num);
    strcat(result, num);
    send_msg(result);
}

/* Test 21: Mixed operations stress test */
void test_21_mixed_operations_stress(void) {
    ensure_hfs_mounted();

    vfs_mkdir("/test/mixed");

    int operations = 0;

    // Mix of operations
    for (int i = 0; i < 10; i++) {
        char base[64];
        strcpy(base, "/test/mixed/item");
        char num[8];
        utoa(i, num);
        strcat(base, num);

        // Every odd: create directory
        if (i % 2 == 1) {
            if (vfs_mkdir(base) == 0) {
                operations++;

                // Add a file in this directory
                char filepath[128];
                strcpy(filepath, base);
                strcat(filepath, "/data.txt");

                if (vfs_create(filepath, 0) == 0) {
                    file_t* f = vfs_open(filepath, 0);
                    if (f) {
                        char data[256];
                        memset(data, 'M', 256);
                        vfs_write(f, data, 256);
                        vfs_close(f);
                        operations++;
                    }
                }
            }
        }
        // Every even: create file
        else {
            strcat(base, ".dat");
            if (vfs_create(base, 0) == 0) {
                file_t* f = vfs_open(base, 0);
                if (f) {
                    char data[512];
                    memset(data, 'F', 512);
                    vfs_write(f, data, 512);
                    vfs_close(f);
                    operations++;
                }
            }
        }
    }

    ASSERT_TRUE(operations >= 10, "mixed operations failed");

    char result[64];
    strcpy(result, "operations:");
    char num[8];
    utoa(operations, num);
    strcat(result, num);
    send_msg(result);
}


// HIDDEN TESTS START HERE

/* ============================================================================
 * HIDDEN TEST 01: Sparse File Write with Random Offsets
 * Test writing to a file at non-contiguous offsets to verify proper block
 * allocation and hole handling
 * ========================================================================== */
void test_h01_sparse_file_random_offsets(void) {

    ensure_hfs_mounted();
    
    vfs_mkdir("/test/sparse");
    vfs_create("/test/sparse/holes.dat", 0);
    
    file_t* file = vfs_open("/test/sparse/holes.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");
    
    // Write at offset 0
    const char* data1 = "START";
    vfs_write(file, (void*)data1, 5);
    
    // Jump to offset 2048 (skip 4 blocks)
    file->f_offset = 2048;
    const char* data2 = "MIDDLE";
    vfs_write(file, (void*)data2, 6);
    
    // Jump to offset 5120 (skip more blocks)
    file->f_offset = 5120;
    const char* data3 = "END";
    vfs_write(file, (void*)data3, 3);
    
    vfs_close(file);
    
    // Verify reads from different offsets
    file = vfs_open("/test/sparse/holes.dat", 0);
    
    char buf1[16], buf2[16], buf3[16];
    memset(buf1, 0, 16);
    memset(buf2, 0, 16);
    memset(buf3, 0, 16);
    
    vfs_read(file, buf1, 5);
    file->f_offset = 2048;
    vfs_read(file, buf2, 6);
    file->f_offset = 5120;
    vfs_read(file, buf3, 3);
    
    vfs_close(file);
    
    bool match = (strcmp(buf1, "START") == 0) && 
                 (strcmp(buf2, "MIDDLE") == 0) && 
                 (strcmp(buf3, "END") == 0);
    
    ASSERT_TRUE(match, "sparse file data mismatch");
    send_msg("PASSED:sparse_offsets");
}

/* ============================================================================
 * HIDDEN TEST 02: Simultaneous Read/Write Operations on Multiple Files
 * Test interleaved operations across multiple files to verify file handle
 * and offset management
 * ========================================================================== */
void test_h02_interleaved_file_operations(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/interleaved");
    
    // Create and open 3 files simultaneously
    vfs_create("/test/interleaved/file_a.txt", 0);
    vfs_create("/test/interleaved/file_b.txt", 0);
    vfs_create("/test/interleaved/file_c.txt", 0);
    
    file_t* file_a = vfs_open("/test/interleaved/file_a.txt", 0);
    file_t* file_b = vfs_open("/test/interleaved/file_b.txt", 0);
    file_t* file_c = vfs_open("/test/interleaved/file_c.txt", 0);
    
    ASSERT_TRUE(file_a != NULL && file_b != NULL && file_c != NULL, "open failed");
    
    // Interleaved writes
    vfs_write(file_a, (void*)"AAA_", 4);
    vfs_write(file_b, (void*)"BBB_", 4);
    vfs_write(file_c, (void*)"CCC_", 4);
    vfs_write(file_a, (void*)"111", 3);
    vfs_write(file_b, (void*)"222", 3);
    vfs_write(file_c, (void*)"333", 3);
    
    vfs_close(file_a);
    vfs_close(file_b);
    vfs_close(file_c);
    
    // Verify each file independently
    file_a = vfs_open("/test/interleaved/file_a.txt", 0);
    char buf[16];
    memset(buf, 0, 16);
    vfs_read(file_a, buf, 7);
    vfs_close(file_a);
    
    bool match = (strcmp(buf, "AAA_111") == 0);
    ASSERT_TRUE(match, "interleaved data mismatch");
    
    send_msg("PASSED:interleaved");
}

/* ============================================================================
 * HIDDEN TEST 03: Maximum File Size with Indirect Pointer Exhaustion
 * Test writing a very large file (30KB+) to stress indirect pointer block
 * and verify proper handling at filesystem limits
 * ========================================================================== */
void test_h03_maximum_file_size(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/maxsize");
    vfs_create("/test/maxsize/huge.bin", 0);
    
    file_t* file = vfs_open("/test/maxsize/huge.bin", 0);
    ASSERT_TRUE(file != NULL, "open failed");
    
    // Write 35KB (70 blocks) - beyond typical indirect pointer capacity
    char block[512];
    int blocks_written = 0;
    
    for (int i = 0; i < 70; i++) {
        // Create unique pattern per block
        for (int j = 0; j < 512; j++) {
            block[j] = (char)((i * 7 + j) & 0xFF);
        }
        block[0] = (char)i; // Marker for block number
        
        int written = vfs_write(file, block, 512);
        if (written == 512) {
            blocks_written++;
        } else {
            break; // Hit filesystem limit
        }
    }
    
    vfs_close(file);
    
    // Verify we can read from the end
    if (blocks_written >= 50) {
        file = vfs_open("/test/maxsize/huge.bin", 0);
        file->f_offset = (blocks_written - 5) * 512;
        
        char verify[512];
        int read_bytes = vfs_read(file, verify, 512);
        
        bool valid = (read_bytes == 512) && (verify[0] == (char)(blocks_written - 5));
        ASSERT_TRUE(valid, "end-of-file verification failed");
        
        vfs_close(file);
    }
    
    ASSERT_TRUE(blocks_written >= 50, "insufficient max file size");
    
    char result[64];
    strcpy(result, "max_blocks:");
    char num[8];
    utoa(blocks_written, num);
    strcat(result, num);
    send_msg(result);
}

/* ============================================================================
 * HIDDEN TEST 04: Directory with Maximum Entry Count
 * Test creating many files in a single directory to stress directory entry
 * management and block chaining for directory data
 * ========================================================================== */
void test_h04_directory_entry_stress(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/maxdir");
    
    int files_created = 0;
    
    // Attempt to create 40 files in one directory
    for (int i = 0; i < 40; i++) {
        char filename[64];
        strcpy(filename, "/test/maxdir/f");
        char num[8];
        utoa(i, num);
        strcat(filename, num);
        strcat(filename, ".dat");
        
        if (vfs_create(filename, 0) == 0) {
            files_created++;
        } else {
            break; // Directory full
        }
    }
    
    ASSERT_TRUE(files_created >= 25, "insufficient directory capacity");
    
    // Verify we can still open files from the middle
    file_t* test_file = vfs_open("/test/maxdir/f25.dat", 0);
    ASSERT_TRUE(test_file != NULL, "cannot open file in full directory");
    vfs_close(test_file);
    
    char result[32];
    strcpy(result, "dir_entries:");
    char num[8];
    utoa(files_created, num);
    strcat(result, num);
    send_msg(result);
}

/* ============================================================================
 * HIDDEN TEST 05: Overwrite Expanding File Across Block Boundaries
 * Test complex overwrite scenarios where file size changes across multiple
 * blocks and offsets
 * ========================================================================== */
void test_h05_complex_overwrite_expansion(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/expand");
    vfs_create("/test/expand/dynamic.dat", 0);
    
    file_t* file = vfs_open("/test/expand/dynamic.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");
    
    // Phase 1: Write 1KB
    char data1[1024];
    for (int i = 0; i < 1024; i++) data1[i] = 'A';
    vfs_write(file, data1, 1024);
    vfs_close(file);
    
    // Phase 2: Overwrite from offset 512, expanding to 2KB total
    file = vfs_open("/test/expand/dynamic.dat", 0);
    file->f_offset = 512;
    char data2[1536];
    for (int i = 0; i < 1536; i++) data2[i] = 'B';
    vfs_write(file, data2, 1536);
    vfs_close(file);
    
    // Phase 3: Overwrite middle portion
    file = vfs_open("/test/expand/dynamic.dat", 0);
    file->f_offset = 1024;
    char data3[512];
    for (int i = 0; i < 512; i++) data3[i] = 'C';
    vfs_write(file, data3, 512);
    vfs_close(file);
    
    // Verify final state: AAAA...BBBB...CCCC...BBBB
    file = vfs_open("/test/expand/dynamic.dat", 0);
    
    char verify[2048];
    memset(verify, 0, 2048);
    int total_read = vfs_read(file, verify, 2048);
    vfs_close(file);
    
    bool valid = (total_read == 2048) &&
                 (verify[0] == 'A') &&        // Start
                 (verify[511] == 'A') &&      // Before overwrite
                 (verify[512] == 'B') &&      // First overwrite
                 (verify[1024] == 'C') &&     // Second overwrite
                 (verify[1536] == 'B');       // After C section
    
    ASSERT_TRUE(valid, "complex overwrite pattern failed");
    send_msg("PASSED:complex_expand");
}

/* ============================================================================
 * HIDDEN TEST 06: Nested Directory Tree with Files at Each Level
 * Create a deep directory hierarchy (8+ levels) with multiple files at each
 * level to test path resolution and inode allocation
 * ========================================================================== */
void test_h06_deep_tree_with_files(void) {
    ensure_hfs_mounted();
    
    // Build path progressively and add files at each level
    char path[256] = "/test/tree";
    vfs_mkdir(path);
    
    int levels_created = 0;
    int total_files = 0;
    
    for (int level = 0; level < 5; level++) {
        strcat(path, "/lv");
        char num[8];
        utoa(level, num);
        strcat(path, num);
        
        if (vfs_mkdir(path) != 0) break;
        levels_created++;
        
        // Create 2 files at this level
        for (int f = 0; f < 2; f++) {
            char filepath[300];
            strcpy(filepath, path);
            strcat(filepath, "/file");
            utoa(f, num);
            strcat(filepath, num);
            strcat(filepath, ".txt");
            
            if (vfs_create(filepath, 0) == 0) {
                total_files++;
                
                // Write level number to file
                file_t* file = vfs_open(filepath, 0);
                if (file) {
                    char data[32];
                    strcpy(data, "Level:");
                    char lnum[8];
                    utoa(level, lnum);
                    strcat(data, lnum);
                    vfs_write(file, data, strlen(data));
                    vfs_close(file);
                }
            }
        }
    }
    
    ASSERT_TRUE(levels_created >= 4, "insufficient tree depth");
    ASSERT_TRUE(total_files >= 8, "insufficient files in tree");
    
    char result[64];
    strcpy(result, "tree:");
    char num[8];
    utoa(levels_created, num);
    strcat(result, num);
    strcat(result, "levels:");
    utoa(total_files, num);
    strcat(result, num);
    strcat(result, "files");
    send_msg(result);
}

/* ============================================================================
 * HIDDEN TEST 07: Fragmented File Write Pattern
 * Write data in alternating small and large chunks to create fragmentation
 * and test block allocation efficiency
 * ========================================================================== */
void test_h07_fragmented_writes(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/frag");
    vfs_create("/test/frag/fragmented.dat", 0);
    
    file_t* file = vfs_open("/test/frag/fragmented.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");
    
    int total_written = 0;
    
    // Alternating pattern: small (100 bytes), large (800 bytes), small, large...
    for (int i = 0; i < 10; i++) {
        // Small write
        char small[100];
        for (int j = 0; j < 100; j++) small[j] = (char)(i + j);
        int w1 = vfs_write(file, small, 100);
        total_written += w1;
        
        // Large write
        char large[800];
        for (int j = 0; j < 800; j++) large[j] = (char)(i * 2 + j);
        int w2 = vfs_write(file, large, 800);
        total_written += w2;
    }
    
    vfs_close(file);
    
    ASSERT_TRUE(total_written >= 8000, "fragmented write failed");
    
    // Verify by reading in different chunk sizes
    file = vfs_open("/test/frag/fragmented.dat", 0);
    
    char verify[9000];
    int total_read = vfs_read(file, verify, 9000);
    vfs_close(file);
    
    ASSERT_TRUE(total_read == total_written, "read/write size mismatch");
    
    char result[64];
    strcpy(result, "fragmented:");
    char num[8];
    utoa(total_written, num);
    strcat(result, num);
    strcat(result, "bytes");
    send_msg(result);
}

/* ============================================================================
 * HIDDEN TEST 08: Multiple Large Files with Concurrent Expansion
 * Create several large files and expand them in an interleaved manner to
 * stress block allocator and indirect pointer management
 * ========================================================================== */
void test_h08_concurrent_large_file_growth(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/concurrent");
    
    const int NUM_FILES = 5;
    const int BLOCKS_PER_ROUND = 3;
    const int ROUNDS = 5;
    
    file_t* files[NUM_FILES];
    int blocks_per_file[NUM_FILES];
    
    // Initialize blocks_per_file array
    for (int i = 0; i < NUM_FILES; i++) {
        blocks_per_file[i] = 0;
    }
    
    // Create files
    for (int i = 0; i < NUM_FILES; i++) {
        char filename[64];
        strcpy(filename, "/test/concurrent/big");
        char num[8];
        utoa(i, num);
        strcat(filename, num);
        strcat(filename, ".dat");
        
        vfs_create(filename, 0);
        files[i] = vfs_open(filename, 0);
        ASSERT_TRUE(files[i] != NULL, "open failed");
    }
    
    // Interleaved writes - grow all files simultaneously
    char block[512];
    for (int round = 0; round < ROUNDS; round++) {
        for (int i = 0; i < NUM_FILES; i++) {
            for (int b = 0; b < BLOCKS_PER_ROUND; b++) {
                // Create unique pattern
                for (int j = 0; j < 512; j++) {
                    block[j] = (char)(i * round * b + j);
                }
                
                if (vfs_write(files[i], block, 512) == 512) {
                    blocks_per_file[i]++;
                }
            }
        }
    }
    
    // Close all files
    for (int i = 0; i < NUM_FILES; i++) {
        vfs_close(files[i]);
    }
    
    // Verify each file
    int files_valid = 0;
    for (int i = 0; i < NUM_FILES; i++) {
        if (blocks_per_file[i] >= 10) {
            files_valid++;
        }
    }
    
    ASSERT_TRUE(files_valid >= 4, "concurrent growth failed");
    
    char result[64];
    strcpy(result, "concurrent:");
    char num[8];
    utoa(files_valid, num);
    strcat(result, num);
    strcat(result, "files");
    send_msg(result);
}

/* ============================================================================
 * HIDDEN TEST 09: Cross-Block Boundary Operations with Verification
 * Perform reads and writes that span multiple block boundaries with various
 * sizes and offsets to test edge cases
 * ========================================================================== */
void test_h09_cross_boundary_edge_cases(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/boundary");
    vfs_create("/test/boundary/edges.dat", 0);
    
    file_t* file = vfs_open("/test/boundary/edges.dat", 0);
    ASSERT_TRUE(file != NULL, "open failed");
    
    // Write 3KB of sequential data
    char write_data[3072];
    for (int i = 0; i < 3072; i++) {
        write_data[i] = (char)(i & 0xFF);
    }
    vfs_write(file, write_data, 3072);
    vfs_close(file);
    
    // Test various boundary-crossing reads
    file = vfs_open("/test/boundary/edges.dat", 0);
    
    // Read 1: Start at 510, read 10 bytes (crosses first block boundary)
    file->f_offset = 510;
    char read1[10];
    int r1 = vfs_read(file, read1, 10);
    
    // Read 2: Start at 1000, read 1100 bytes (crosses 2 boundaries)
    file->f_offset = 1000;
    char read2[1100];
    int r2 = vfs_read(file, read2, 1100);
    
    // Read 3: Start at 2560, read 512 bytes (crosses into potential next block)
    file->f_offset = 2560;
    char read3[512];
    int r3 = vfs_read(file, read3, 512);
    
    vfs_close(file);
    
    // Verify data integrity
    bool valid = (r1 == 10) && (r2 == 1100) && (r3 == 512);
    
    if (valid) {
        // Check actual data matches
        for (int i = 0; i < 10; i++) {
            if (read1[i] != write_data[510 + i]) valid = false;
        }
        for (int i = 0; i < 1100; i++) {
            if (read2[i] != write_data[1000 + i]) valid = false;
        }
        for (int i = 0; i < 512; i++) {
            if (read3[i] != write_data[2560 + i]) valid = false;
        }
    }
    
    ASSERT_TRUE(valid, "boundary crossing data mismatch");
    send_msg("PASSED:boundary_edges");
}

/* ============================================================================
 * HIDDEN TEST 10: Stress Test - Mixed Operations at Scale
 * Combine directory creation, file creation, large writes, and random access
 * in a complex scenario to stress all filesystem components
 * ========================================================================== */
void test_h10_comprehensive_stress_test(void) {
    ensure_hfs_mounted();
    
    vfs_mkdir("/test/stress");
    
    int operations = 0;
    int dirs_created = 0;
    int files_created = 0;
    int large_writes = 0;
    
    // Phase 1: Create directory structure
    for (int i = 0; i < 8; i++) {
        char dirname[64];
        strcpy(dirname, "/test/stress/dir");
        char num[8];
        utoa(i, num);
        strcat(dirname, num);
        
        if (vfs_mkdir(dirname) == 0) {
            dirs_created++;
            operations++;
            
            // Phase 2: Create files in each directory
            for (int f = 0; f < 3; f++) {
                char filepath[128];
                strcpy(filepath, dirname);
                strcat(filepath, "/file");
                char fnum[8];
                utoa(f, fnum);
                strcat(filepath, fnum);
                strcat(filepath, ".dat");
                
                if (vfs_create(filepath, 0) == 0) {
                    files_created++;
                    operations++;
                    
                    // Phase 3: Write varying amounts of data
                    file_t* file = vfs_open(filepath, 0);
                    if (file) {
                        // Vary size: 500 bytes to 2KB
                        int size = 500 + (i * f * 100);
                        if (size > 2048) size = 2048;
                        
                        char data[2048];
                        for (int d = 0; d < size; d++) {
                            data[d] = (char)((i + f + d) & 0xFF);
                        }
                        
                        if (vfs_write(file, data, size) == size) {
                            operations++;
                            if (size >= 1024) large_writes++;
                        }
                        
                        vfs_close(file);
                    }
                }
            }
        }
    }
    
    // Phase 4: Random access verification
    file_t* verify_file = vfs_open("/test/stress/dir5/file1.dat", 0);
    if (verify_file) {
        verify_file->f_offset = 100;
        char buf[50];
        if (vfs_read(verify_file, buf, 50) == 50) {
            operations++;
        }
        vfs_close(verify_file);
    }
    
    ASSERT_TRUE(dirs_created >= 6, "insufficient directories");
    ASSERT_TRUE(files_created >= 15, "insufficient files");
    ASSERT_TRUE(operations >= 25, "insufficient operations");
    
    char result[128];
    strcpy(result, "stress:dirs:");
    char num[8];
    utoa(dirs_created, num);
    strcat(result, num);
    strcat(result, ":files:");
    utoa(files_created, num);
    strcat(result, num);
    strcat(result, ":ops:");
    utoa(operations, num);
    strcat(result, num);
    send_msg(result);
}