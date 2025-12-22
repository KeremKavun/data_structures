#ifndef STACK_LSTACK_H
#define STACK_LSTACK_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../../concepts/include/allocator_concept.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Stack ADT with linked list data structure
 * 
 * @brief Stack ADT.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct lstack *ls` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal nodes are owned by the underlying list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. lstack_deinit might be helpful to destruct remaining
 * - objects in the stack.
 * @{
 */

/**
 * @struct lstack
 * 
 * @brief Stack ADT with singular linked list.
 * 
 * @warning **Null Safety**: All functions taking `struct lstack *` 
 * expect a valid, initialized by @ref lstack_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct lstack {
    struct slist        *contents;
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the stack.
 * @{
 */

/**
 * @brief Initializes the stack ADT.
 * 
 * @param[in, out] ls Pointer to the stack instance.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes. Must not be NULL or invalid.
 * 
 * @return 0 on success, non-zero otherwise.
 * 
 * @see allocator_concept
 * @see slist_init
 */
int lstack_init(struct lstack* ls, struct allocator_concept* ac);

/**
 * @brief Deinitializes the stack ADT.
 * 
 * @param[in, out] ls Pointer to the stack instance.
 * @param[in] oc Pointer to an object_concept used to deinit objects.
 * 
 * @see object_concept
 */
void lstack_deinit(struct lstack* ls, struct object_concept* oc);

/** @} */ // End of Initialization & Deinitialization group

/**
 * @name Push & Pop
 * Functions to push and pop items.
 * @{
 */

/**
 * @brief Pushes new item.
 * 
 * @param[in] new_item new item to be pushed into the stack.
 * 
 * @return 0 on success, non-zero otherwise.
 */
int lpush(struct lstack* ls, void* new_item);

/**
 * @brief Pops the stack ADT.
 * 
 * @return Reference to an object, supplied by user,
 * stored in the stack
 */
void* lpop(struct lstack* ls);

/** @} */ // End of Push & Pop group

/**
 * @name Inspection
 * Functions to query stack ADT.
 * @{
 */

/**
 * @return Reference to the data stored at the top,
 * without popping.
 */
void* ltop(struct lstack* ls);

/**
 * @return 1 if the stack is empty, 0 otherwise.
 */
int lstack_empty(const struct lstack* ls);

/**
 * @return Count of the objects stored in the stack.
 */
size_t lstack_size(const struct lstack* ls);

/** @} */ // End of Inspection group

/**
 * @brief Iterates over the stack.
 * 
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void lstack_walk(struct lstack* ls, void* context, void (*handler) (void* item, void* context));

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // STACK_LSTACK_H