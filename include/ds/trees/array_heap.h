#ifndef TREES_ARRAY_HEAP_H
#define TREES_ARRAY_HEAP_H

#include <ds/utils/object_concept.h>
#include <ds/utils/debug.h>
#include <ds/arrays/dynarray.h>
#include "common/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file array_heap.h
 * @brief Defines the interface for array-based heaps.
 */

/**
 * @defgroup BINHEAP Binary Heap (with array)
 * @ingroup BINTREE_CORE
 * @brief Heap data structure with dynarray.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct array_heap *tree` arguments must be non-NULL.
 * - **Ownership**: Internal slots are owned by underlying dynarray (vector) and managed thru object_concept
 * - given by user to init and deinit objects in place.
 * @{
 */

struct array_heap {
    struct dynarray     contents;                   ///< Used to manage memory automatically.
    int (*cmp) (const void *a, const void *b);      ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b.
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the array_heap.
 * @{
 */

/**
 * @brief Initializes the array_heap.
 * @param[in, out] tree Pointer to the array_heap instance.
 * @param[in] obj_size Object size of the type to be stored.
 * @param[in] oc copy concept of the object type to be stored.
 * Must be non-NULL and valid.
 * @param[in] cmp Function pointer to compare keys. if a > b gives
 * positiv, then you get max array_heap; if opposite, you get min array_heap.
 * @return 0 on success, non-zero otherwise.
 * @see dynarray
 */
int array_heap_init(struct array_heap *tree, size_t obj_size, struct object_concept *oc, int (*cmp) (const void *a, const void *b));

/** @brief Deinits the array_heap. */
void array_heap_deinit(struct array_heap *tree);

/** @} */ // End of Initialize & Deinitialize

/**
 * @name Operations
 * Functions for add, remove.
 * @{
 */

/**
 * @brief Adds new item into the array_heap, copying or emplacing.
 * @param[in] new_data Data to be placed.
 * @return 0 on success, non-zero otherwise.
 */
int array_heap_add(struct array_heap *tree, void *new_data);

/**
 * @brief Removes
 * @param[out] removed Memory block that is able to hold type
 * of the object the array_heap stores.
 * @return 0 on success, non-zero otherwise.
 */
int array_heap_remove(struct array_heap *tree, void *removed);

/** @} */ // End of Operations

/**
 * @name Inspection
 * Functions to query the array_heap.
 * @{
 */

/** @return 1 if the array_heap is empty, 0 otherwise. */
int array_heap_empty(const struct array_heap *tree);

/** @return Count of the objects stored in the array_heap. */
size_t array_heap_size(const struct array_heap *tree);

/** @} */ // End of Inspection

/**
 * @brief Iterates over the array_heap.
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void array_heap_walk(struct array_heap *tree, void *context, void (*handler) (void *data, void *context));

/** @} */ // End of BINHEAP group

#ifdef __cplusplus
}
#endif

#endif // TREES_ARRAY_HEAP_H