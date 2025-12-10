#ifndef __LIBC_BITMAP_H
#define __LIBC_BITMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//! some defines corresponding to the bitset/bitmap wtv
#define BITSET_WORD      uint32_t
#define BITSET_WORD_SIZE (sizeof(BITSET_WORD) * 8)

//! useful utilities to manipulate the bitmap as needed
static inline void bitmap_set(BITSET_WORD* bm, int32_t idx) {
    bm[ idx / BITSET_WORD_SIZE ] |=  ((BITSET_WORD)1 << (idx % BITSET_WORD_SIZE));
}

static inline void bitmap_unset(BITSET_WORD* bm, int32_t idx) {
    bm[ idx / BITSET_WORD_SIZE ] &= ~((BITSET_WORD)1 << (idx % BITSET_WORD_SIZE));
}

static inline bool bitmap_test(BITSET_WORD* bm, int32_t idx) {
    return (bm[ idx / BITSET_WORD_SIZE ] &   ((BITSET_WORD)1 << (idx % BITSET_WORD_SIZE)));
}

//! finds the start of the first free block in the bitmap
static int32_t bitmap_first_free(BITSET_WORD* bm, uint32_t max_bits) {

    int32_t idx = -1;
    for (uint32_t i = 0; i < max_bits; i++) {
        if (!bitmap_test(bm, i)) {
            idx = i;
            break;
        }
    }

    return idx;
}

//! finds the start of the first "n" consecutive free blocks in the bitmap
// static int32_t bitmap_first_n_free(BITSET_WORD* bm, uint32_t max_bits, uint32_t n);

// //! finds the start of the first free block after the given index
// static int32_t bitmap_first_free_after(BITSET_WORD* bm, uint32_t max_bits, uint32_t start_idx);

// //! finds the start of the first "n" consecutive free blocks after the given index
// static int32_t bitmap_first_n_free_after(BITSET_WORD* bm, uint32_t max_bits, uint32_t start_idx, uint32_t n);

#endif // __LIBC_BITMAP_H