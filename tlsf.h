/*
 * SPDX-FileCopyrightText: 2006-2016 Matthew Conte
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf

#include <stddef.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* tlsf_t: a TLSF structure. Can contain 1 to N pools. */
/* pool_t: a block of memory that TLSF can manage. */
typedef void* tlsf_t;
typedef void* pool_t;

/* Create/destroy a memory pool. */
tlsf_t tlsf_create(void* mem);
tlsf_t tlsf_create_with_pool(void* mem, size_t bytes);
void tlsf_destroy(tlsf_t tlsf);
pool_t tlsf_get_pool(tlsf_t tlsf);

/* Add/remove memory pools. */
pool_t tlsf_add_pool(tlsf_t tlsf, void* mem, size_t bytes);
void tlsf_remove_pool(tlsf_t tlsf, pool_t pool);

/* malloc/memalign/realloc/free replacements. */
void* tlsf_malloc(tlsf_t tlsf, size_t size);
void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size);
void* tlsf_memalign_offs(tlsf_t tlsf, size_t align, size_t size, size_t offset);
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size);
void tlsf_free(tlsf_t tlsf, void* ptr);

/* Returns internal block size, not original request size */
size_t tlsf_block_size(void* ptr);

/* Overheads/limits of internal structures. */
size_t tlsf_size(void);
size_t tlsf_align_size(void);
size_t tlsf_block_size_min(void);
size_t tlsf_block_size_max(void);
size_t tlsf_pool_overhead(void);
size_t tlsf_alloc_overhead(void);

/* Debugging. */
typedef void (*tlsf_walker)(void* ptr, size_t size, int used, void* user);
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void* user);
/* Returns nonzero if any internal consistency check fails. */
int tlsf_check(tlsf_t tlsf);
int tlsf_check_pool(pool_t pool);

/*!
 * @brief Weak function filling the given memory with a given fill pattern.
 * 
 * @param start: pointer to the start of the memory region to fill
 * @param size: size of the memory region to fill
 * @param is_free: Indicate if the pattern to use the fill the region should be 
 * an after free or after allocation pattern.
 */
__attribute__((weak)) void block_absorb_post_hook(void *start, size_t size, bool is_free);

#if defined(__cplusplus)
};
#endif

#endif
