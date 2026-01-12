#ifndef STACK_VSTACK_H
#define STACK_VSTACK_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Stack ADT with dynamic array.
 * 
 * @brief Stack ADT.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct vstack *vs` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal slots are owned by underlying dynarray (vector) and managed thru object_concept
 * - given by user to init and deinit objects in place.
 * @{
 */

/**
 * @struct vstack
 * 
 * @brief Stack ADT with dynarray.
 * 
 * @warning **Null Safety**: All functions taking `struct vstack *` 
 * expect a valid, initialized by @ref vstack_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct vstack;

/**
 * @name Create & Destroy
 * Functions for setting up the stack.
 * @{
 */

/**
 * @brief Creates the stack ADT.
 * 
 * @param[in] obj_size Size of object type to be stored.
 * @param[in] oc copy concept of object to init and deinit objects.
 * 
 * @return vstack, NULL otherwise.
 * 
 * @see object_concept
 * @see dynarray_init
 */
struct vstack *vstack_create(size_t obj_size, struct object_concept *oc);

/**
 * @brief Destroy the stack ADT.
 * 
 * @param[in, out] vs Pointer to the stack instance.
 */
void vstack_destroy(struct vstack *vs);

/** @} */ // End of Create & Destroy group

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
int vpush(struct vstack *vs, void *new_item);

/**
 * @brief Pops the stack ADT.
 * 
 * @param[in, out] popped_item Pointer to memory block to
 * store popped item. Nothing is copied if NULL.
 * 
 * @return 0 on success, positive if copy ctor fails, negative
 * if the stack is empty.
 */
int vpop(struct vstack *vs, void *popped_item);

/** @} */ // End of Push & Pop group

/**
 * @name Inspection
 * Functions to query stack ADT.
 * @{
 */

/**
 * @param[in, out] top_item Pointer to memory block to
 * store top item.
 * 
 * @return 0 on success, positive if copy ctor fails, negative
 * if the stack is empty.
 */
int vtop(struct vstack *vs, void *top_item);

/**
 * @return 1 if the stack is empty, 0 otherwise.
 */
int vstack_empty(const struct vstack *vs);

/**
 * @return Count of the objects stored in the stack.
 */
size_t vstack_size(const struct vstack *vs);

/** @} */ // End of Inspection group

/**
 * @brief Iterates over the stack.
 * 
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void vstack_walk(struct vstack *vs, void *context, void (*handler) (void *item, void *context));

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // STACK_VSTACK_H