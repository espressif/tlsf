/*
 * SPDX-FileCopyrightText: 2006-2016 Matthew Conte
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once
#include <stddef.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
** Constants definition for poisoning.
** These defines are used as 3rd argument of tlsf_poison_fill_region() for readability purposes.
*/
#define POISONING_AFTER_FREE true
#define POISONING_AFTER_MALLOC !POISONING_AFTER_FREE

/*
** Cast and min/max macros.
*/
#define tlsf_cast(t, exp)	((t) (exp))
#define tlsf_min(a, b)		((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)		((a) > (b) ? (a) : (b))

/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert assert
#endif

enum tlsf_config
{
	/* log2 of number of linear subdivisions of block sizes. Larger
	** values require more memory in the control structure. Values of
	** 4 or 5 are typical, 3 is for very small pools.
	*/
	SL_INDEX_COUNT_LOG2_MIN  = 3,

	/* All allocation sizes and addresses are aligned to 4 bytes. */
	ALIGN_SIZE_LOG2 = 2,
	ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

	/*
	** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
	** However, because we linearly subdivide the second-level lists, and
	** our minimum size granularity is 4 bytes, it doesn't make sense to
	** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
	** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
	** trying to split size ranges into more slots than we have available.
	** Instead, we calculate the minimum threshold size, and place all
	** blocks below that size into the 0th first-level list.
	** Values below are the absolute minimum to accept a pool addition.    
	*/

	/* Tunning the first level, we can reduce TLSF pool overhead
	 * in exchange of manage a pool smaller than 4GB
	 */
	FL_INDEX_MAX_MIN = 14,
	SL_INDEX_COUNT_MIN = (1 << SL_INDEX_COUNT_LOG2_MIN),
	FL_INDEX_SHIFT_MIN = (SL_INDEX_COUNT_LOG2_MIN + ALIGN_SIZE_LOG2),
	FL_INDEX_COUNT_MIN = (FL_INDEX_MAX_MIN - FL_INDEX_SHIFT_MIN + 1),
};

/*
** Data structures and associated constants.
*/

/* A type used for casting when doing pointer arithmetic. */
typedef ptrdiff_t tlsfptr_t;

typedef struct block_header_t
{
	/* Points to the previous physical block. */
	struct block_header_t* prev_phys_block;

	/* The size of this block, excluding the block header. */
	size_t size;

	/* Next and previous free blocks. */
	struct block_header_t* next_free;
	struct block_header_t* prev_free;
} block_header_t;

/*
** Since block sizes are always at least a multiple of 4, the two least
** significant bits of the size field are used to store the block status:
** - bit 0: whether block is busy or free
** - bit 1: whether previous block is busy or free
*/
static const size_t block_header_free_bit = 1 << 0;
static const size_t block_header_prev_free_bit = 1 << 1;

/*
** The size of the block header exposed to used blocks is the size field.
** The prev_phys_block field is stored *inside* the previous free block.
*/
static const size_t block_header_overhead = sizeof(size_t);

/* User data starts directly after the size field in a used block. */
static const size_t block_start_offset =
	offsetof(block_header_t, size) + sizeof(size_t);

/*
** A free block must be large enough to store its header minus the size of
** the prev_phys_block field, and no larger than the number of addressable
** bits for FL_INDEX.
*/
static const size_t block_size_min = 
	sizeof(block_header_t) - sizeof(block_header_t*);
static const size_t block_size_max = tlsf_cast(size_t, 1) << FL_INDEX_MAX_MIN;

/* The TLSF control structure. */
typedef struct control_t
{
    /* Empty lists point at this block to indicate they are free. */
    block_header_t block_null;

    /* Local parameter for the pool */
    unsigned int fl_index_count;
    unsigned int fl_index_shift;
    unsigned int fl_index_max;  
    unsigned int sl_index_count;
    unsigned int sl_index_count_log2;
    unsigned int small_block_size;
    size_t size;

    /* Bitmaps for free lists. */
    unsigned int fl_bitmap;
    unsigned int *sl_bitmap;    

    /* Head of free lists. */
    block_header_t** blocks;
} control_t;


#if defined(__cplusplus)
};
#endif
