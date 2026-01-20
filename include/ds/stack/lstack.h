#ifndef STACK_LSTACK_H
#define STACK_LSTACK_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/allocator_concept.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file lstack.h
 * @brief Defines the interface for List Stack.
 */

/**
 * @defgroup LSTACK List Stack
 * @ingroup STACK
 * @brief Implementation using a singular linked list. Best for sparse memory or unknown growth.
 * 
 * @details 
 * ### Global Constraints
 * - **NULL Pointers**: All `struct lstack *ls` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal nodes are owned by the underlying list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. lstack_deinit might be helpful to destruct remaining
 * - objects in the stack.
 * * @{
 */

/**
 * @struct lstack
 * @brief Stack ADT with singular linked list.
 * @details **Ownership**: Internal nodes are owned by the underlying list and 
 * managed by `allocator_concept`.
 */
struct lstack;

/**
 * @name Create & Destroy
 * Functions for setting up the stack.
 * @{
 */

/**
 * @brief Creates the stack ADT.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes.
 * Use @ref lstack_node_sizeof to pass object size into your allocator that will allocate lstack nodes.
 * @return lstack if successfull, NULL otherwise
 * @see allocator_concept
 * @see slist_init
 */
struct lstack *lstack_create(struct allocator_concept *ac);

/**
 * @brief Destroys the stack ADT.
 * @param[in, out] ls Pointer to the stack instance.
 * @param[in] deinit Pointer to func pointer used to deinit objects.
 * Might be NULL.
 * @see object_concept
 */
void lstack_destroy(struct lstack *ls, deinit_cb deinit);

/** @return Size of the node in bytes allocated by allocator_concept */
size_t lstack_node_sizeof();

/** @} */ // End of Create & Destroy

/**
 * @name Push & Pop
 * Functions to push and pop items.
 * @{
 */

/**
 * @brief Pushes new item.
 * @param[in] new_item new item to be pushed into the stack.
 * @return 0 on success, non-zero otherwise.
 */
int lpush(struct lstack *ls, void *new_item);

/**
 * @brief Pops the stack ADT.
 * @return Reference to an object, supplied by user,
 * stored in the stack
 */
void *lpop(struct lstack *ls);

/** @} */ // End of Push & Pop

/**
 * @name Inspection
 * Functions to query stack ADT.
 * @{
 */

/**
 * @return Reference to the data stored at the top,
 * without popping.
 */
void *ltop(struct lstack *ls);

/** @return 1 if the stack is empty, 0 otherwise. */
int lstack_empty(const struct lstack *ls);

/** @return Count of the objects stored in the stack. */
size_t lstack_size(const struct lstack *ls);

/** @} */ // End of Inspection

/**
 * @brief Iterates over the stack.
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void lstack_walk(struct lstack *ls, void *context, void (*handler) (void *item, void *context));

/** @} */ // End of LSTACK group

#ifdef __cplusplus
}
#endif

#endif // STACK_LSTACK_H