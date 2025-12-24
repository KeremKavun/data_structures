#ifndef TREES_HEAP_H
#define TREES_HEAP_H

#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include "../internals/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Heap ADT with dynarray
 * 
 * @brief Basic operations for heaps.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct heap *tree` arguments must be non-NULL.
 * - **Ownership**: Internal slots are owned by underlying dynarray (vector) and managed thru object_concept
 * - given by user to init and deinit objects in place.
 * @{
 */

struct heap {
    struct dynarray     *contents;
    int (*cmp) (const void *a, const void *b);
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the heap.
 * @{
 */

/**
 * @brief Initializes the heap.
 * 
 * @param[in, out] tree Pointer to the heap instance.
 * @param[in] obj_size Object size of the type to be stored.
 * @param[in] oc copy concept of the object type to be stored.
 * Must be non-NULL and valid.
 * @param[in] cmp Function pointer to compare keys. if a > b gives
 * positiv, then you get max heap; if opposite, you get min heap.
 * 
 * @return 0 on success, non-zero otherwise.
 * 
 * @see dynarray
 */
int heap_init(struct heap *tree, size_t obj_size, struct object_concept *oc, int (*cmp) (const void *a, const void *b));

/**
 * @brief Deinits the heap.
 */
void heap_deinit(struct heap *tree);

/** @} */ // End of Initialize & Deinitialize group

/**
 * @name Operations
 * Functions for add, remove.
 * @{
 */

/**
 * @brief Adds new item into the heap, copying or emplacing.
 * 
 * @param[in] new_data Data to be placed.
 * 
 * @return 0 on success, non-zero otherwise.
 */
int heap_add(struct heap *tree, void *new_data);

/**
 * @brief Removes 
 * 
 * @param[out] removed Memory block that is able to hold type
 * of the object the heap stores.
 * 
 * @return 0 on success, non-zero otherwise.
 */
int heap_remove(struct heap *tree, void *removed);

/** @} */ // End of Operations group

/**
 * @name Inspection
 * Functions to query the heap.
 * @{
 */

/**
 * @return 1 if the heap is empty, 0 otherwise.
 */
int heap_empty(const struct heap *tree);

/**
 * @return Count of the objects stored in the heap.
 */
size_t heap_size(const struct heap *tree);

/** @} */ // End of Inspection group

/**
 * @brief Iterates over the heap.
 * 
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void heap_walk(struct heap *tree, void *context, void (*handler) (void *data, void *context));

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // TREES_HEAP_H