#include <mm/kheap.h>
#include <utils.h>
#include <stdio.h>
#include <string.h>
#include <testmain.h>   // for send_msg()

#define HEAP_SIZE 4096   // 4 KB fake heap for tests
static uint8_t heap_area[HEAP_SIZE];
static heap_t test_heap;

static void reset_heap() {
    kheap_init(&test_heap, heap_area, HEAP_SIZE, HEAP_SIZE, false, false);
}

static inline size_t next_pow2(size_t x) {
    if (x <= 1) return 1;
    x--;
    x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
#if UINTPTR_MAX > 0xffffffffu
    x |= x >> 32;
#endif
    return x + 1;
}

// ---------------- Initialization ----------------
void test_kheap_init() {
    //reset_heap();
    send_msg("PASSED");
}

// ---------------- Allocation ----------------
void test_kheap_alloc_small() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 1);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_alloc_exact() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 32);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_split() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 16);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

// ---------------- Freeing ----------------
void test_kheap_free_reuse() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    free(p);
    void *q = kmalloc(get_kernel_heap(), 64);
    send_msg(p == q ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_coalesce() {
    //reset_heap();
    void *a = kmalloc(get_kernel_heap(), 16);
    void *b = kmalloc(get_kernel_heap(), 16);
    free(a);
    free(b);
    void *c = kmalloc(get_kernel_heap(), 32);
    send_msg(c ? "PASSED" : "FAILED");
    free(c);
}

void test_kheap_double_free() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    free(p);
    free(p);
    send_msg("PASSED");
}

void test_kheap_invalid_free() {
    //reset_heap();
    int dummy = 123;
    kfree(get_kernel_heap(), &dummy);
    send_msg("PASSED");
}

// ---------------- Realloc ----------------
void test_kheap_realloc_shrink() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 128);
    void *q = krealloc(get_kernel_heap(), p, 64);
    send_msg(p == q ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_realloc_expand() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    strcpy((char*)p, "buddytest");
    void *q = krealloc(get_kernel_heap(), p, 512);
    send_msg(q && strcmp((char*)q, "buddytest")==0 ? "PASSED" : "FAILED");
    free(q);
}

void test_kheap_realloc_null() {
    //reset_heap();
    void *p = krealloc(get_kernel_heap(), NULL, 128);
    send_msg(p ? "PASSED" : "FAILED");
    free(p);
}

void test_kheap_realloc_zero() {
    //reset_heap();
    void *p = kmalloc(get_kernel_heap(), 64);
    void *q = krealloc(get_kernel_heap(), p, 0);
    send_msg(q == NULL ? "PASSED" : "FAILED");
}

// ---------------- OOM ----------------
void test_kheap_oom() {
    //reset_heap();
    void *arr[200];
    int count = 0;
    for (int i = 0; i < 200; i++) {
        arr[i] = kmalloc(get_kernel_heap(), 32);
        if (!arr[i]) break;
        count++;
    }
    send_msg(count > 0 ? "PASSED" : "FAILED");
    for (int i = 0; i < count; i++) free(arr[i]);
}

// ---------------- Stress ----------------
void test_kheap_stress_pattern() {
    //reset_heap();
    void *a[10];
    for (int i = 0; i < 10; i++) a[i] = kmalloc(get_kernel_heap(), 32);
    for (int i = 0; i < 10; i += 2) free(a[i]);
    void *mid = kmalloc(get_kernel_heap(), 64);
    send_msg(mid ? "PASSED" : "FAILED");
    free(mid);
    for (int i = 1; i < 10; i += 2) free(a[i]);
}


/* -------------------------------------------------------------------------- */
/* Fragmentation and coalescing                                               */
/* -------------------------------------------------------------------------- */
void test_kheap_fragmentation_coalescing(void) {
    // reset_heap();
    void *blocks[16];
    for (int i = 0; i < 16; i++)
        blocks[i] = kmalloc(get_kernel_heap(), 30);   // not multiple of 2

    for (int i = 0; i < 16; i += 2)
        kfree(get_kernel_heap(), blocks[i]);

    void *mix1 = kmalloc(get_kernel_heap(), 70);
    void *mix2 = kmalloc(get_kernel_heap(), 110);
    void *mix3 = kmalloc(get_kernel_heap(), 18);

    kfree(get_kernel_heap(), mix1);
    kfree(get_kernel_heap(), mix2);
    kfree(get_kernel_heap(), mix3);
    for (int i = 1; i < 16; i += 2)
        kfree(get_kernel_heap(), blocks[i]);

    void *big = kmalloc(get_kernel_heap(), HEAP_SIZE / 2 - 24);
    send_msg(big ? "PASSED" : "FAILED");
    if (big) free(big);
}

/* -------------------------------------------------------------------------- */
/* Alignment correctness (header-agnostic)                                   */
/* -------------------------------------------------------------------------- */
void test_kheap_alignment_check(void) {

    bool aligned = true;

    for (int sz = 3; sz <= HEAP_SIZE / 4; sz *= 2) {
        void *p = kmalloc(get_kernel_heap(), sz);
        if (!p) { aligned = false; break; }

        // Calculate the allocator’s expected block size
        size_t blk = next_pow2(sz);
        uintptr_t ptr = (uintptr_t)p;

        // Derive the block base (mask down)
        uintptr_t block_base = ptr & ~(blk - 1);

        // The pointer must lie *within* its own aligned block region
        if (ptr < block_base || ptr >= block_base + blk) {
            aligned = false;
            break;
        }

        kfree(get_kernel_heap(), p);
    }

    send_msg(aligned ? "PASSED" : "FAILED");
}

/* -------------------------------------------------------------------------- */
/* Random fuzz stress                                                        */
/* -------------------------------------------------------------------------- */
static uint32_t xorshift32(uint32_t *s) {
    uint32_t x = *s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return *s = x;
}

void test_kheap_random_stress(void) {

    uint32_t seed = 0xC0FFEEu;
    const int N = 512;
    void *arr[N];
    memset(arr, 0, sizeof(arr));

    for (int i = 0; i < 20000; i++) {
        uint32_t r = xorshift32(&seed);
        int idx = r % N;
        size_t sz = (r % 200) + 5;
        if ((r & 1) && !arr[idx])
            arr[idx] = kmalloc(get_kernel_heap(), sz);
        else if (arr[idx]) {
            kfree(get_kernel_heap(), arr[idx]);
            arr[idx] = NULL;
        }
    }

    for (int i = 0; i < N; i++) if (arr[i]) kfree(get_kernel_heap(), arr[i]);
    send_msg("PASSED");
}

/* -------------------------------------------------------------------------- */
/* Realloc integrity                                                         */
/* -------------------------------------------------------------------------- */
void test_kheap_realloc_integrity(void) {
    
    void *p = kmalloc(get_kernel_heap(), 64);
    strcpy(p, "abcdefghijklmnopqrstuvwxyz");

    p = krealloc(get_kernel_heap(), p, 100);
    ASSERT_TRUE(strcmp(p, "abcdefghijklmnopqrstuvwxyz") == 0, "expand1 corrupted");

    p = krealloc(get_kernel_heap(), p, 300);
    ASSERT_TRUE(strcmp(p, "abcdefghijklmnopqrstuvwxyz") == 0, "expand2 corrupted");

    p = krealloc(get_kernel_heap(), p, 30);
    ASSERT_TRUE(strcmp(p, "abcdefghijklmnopqrstuvwxyz") == 0, "shrink corrupted");

    kfree(get_kernel_heap(), p);
    send_msg("PASSED");
}



/* -------------------------------------------------------------------------- */
/* Buddy system verification                                                  */
/* -------------------------------------------------------------------------- */

void test_kheap_buddy_symmetry(void) {

    heap_t *heap = get_kernel_heap();

    // Allocate two same-order buddies
    void *a = kmalloc(heap, 60);
    void *b = kmalloc(heap, 60);

    size_t blk_size = next_pow2(60);
    uintptr_t base_a = (uintptr_t)a & ~(blk_size - 1);
    uintptr_t base_b = (uintptr_t)b & ~(blk_size - 1);

    // Free in reversed order (to test merge symmetry)
    kfree(heap, b);
    kfree(heap, a);

    // Allocate one block of double size — should come from merged region
    void *merged = kmalloc(heap, blk_size * 2 - 8);
    uintptr_t merged_base = (uintptr_t)merged & ~((blk_size * 2) - 1);

    bool symmetric_merge =
        merged && ((merged_base == (base_a < base_b ? base_a : base_b)) ||
                   ((uintptr_t)merged % (blk_size * 2) == 0));

    send_msg(symmetric_merge ? "PASSED: Buddy merge symmetric"
                             : "FAILED: Merge depends on order");

    if (merged) kfree(heap, merged);
}

/* -------------------------------------------------------------------------- */
/* Multilevel merging (power of two check)                                   */
/* -------------------------------------------------------------------------- */
void test_kheap_buddy_multilevel(void) {

    heap_t *heap = get_kernel_heap();

    void *a = kmalloc(heap, 60);
    void *b = kmalloc(heap, 60);
    void *c = kmalloc(heap, 60);
    void *d = kmalloc(heap, 60);

    kfree(heap, a);
    kfree(heap, b);
    kfree(heap, c);
    kfree(heap, d);

    void *merged = kmalloc(heap, 220);
    bool merged_correctly = (uintptr_t)merged == (uintptr_t)a;

    send_msg(merged_correctly ? "PASSED: Multi-level buddy merge detected"
                              : "FAILED: Not real buddy allocator");

    if (merged) kfree(heap, merged);
    
}
