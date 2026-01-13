#ifndef ALLOCATOR_CONCEPT_H
#define ALLOCATOR_CONCEPT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// WARNING! This works in *86 and ARM architectures, but does not conform c standart.
#define GENERIC_ALLOC_SIGN void* (*) (void *)
// WARNING! This works in *86 and ARM architectures, but does not conform c standart.
#define GENERIC_FREE_SIGN void (*) (void *, void *)

/**
 * @defgroup ALLOC_CONCEPT Allocator Concept
 * @ingroup UTILS
 * @brief Metadata for memory pool management across the codebase.
 * @{
 */

/**
 * @struct allocator_concept
 * @brief Stores function pointers for custom memory allocation logic.
 * This structure allows containers to use different memory strategies (e.g., arena, 
 * pool, or heap) interchangeably.
 * @warning You must provide a valid @p allocator context, @p alloc, and @p free 
 * functions to avoid segmentation faults.
 * @note **Platform Support**: Signatures match x86 and ARM pointer sizes but 
 * may require wrappers for strict C standard compliance on other architectures.
 */
struct allocator_concept {
    void *allocator;                                ///< Pointer to the specific pool or context.
    void *(*alloc) (void *allocator);               ///< Allocation function pointer.
    void (*free) (void *allocator, void *ptr);      ///< Deallocation function pointer.
};

/**
 * @struct syspool
 * @brief Standard library wrapper for `malloc` and `free`.
 * Use this when a specialized memory pool is not required.
 */
struct syspool {
    size_t obj_size; ///< Size of the objects to be allocated.
};

/** @brief Malloc wrapper for @ref syspool. */
void *sysalloc(void *allocator);

/** @brief Free wrapper for @ref syspool. */
void sysfree(void *allocator, void *ptr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // ALLOCATOR_CONCEPT_H