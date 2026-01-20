#ifndef QUEUE_LQUEUE_H
#define QUEUE_LQUEUE_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/allocator_concept.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file lqueue.h
 * @brief Defines the interface for List Queue.
 */

/**
 * @defgroup LQUEUE List Queue
 * @ingroup QUEUE
 * @brief Implements queue ADT using singly linked list.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct queue *lq` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal nodes are owned by the underlying list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. lqueue_deinit might be helpful to destruct remaining
 * - objects in the queue.
 * @{
 */

/**
 * @struct lqueue
 * @brief Queue ADT with singular linked list.
 * 
 * @details **Ownership**: Internal nodes are owned by the underlying list and 
 * managed by `allocator_concept`.
 */
struct lqueue;

/**
 * @name Create & Destroy
 * Functions for setting up the queue.
 * @{
 */

/**
 * @brief Creates the queue ADT.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes. Must not be NULL or invalid.
 * Use @ref lqueue_node_sizeof to pass object size into your allocator that will allocate lqueue nodes.
 * @return lqueue, NULL otherwise.
 * @see allocator_concept
 * @see slist_init
 */
struct lqueue *lqueue_create(struct allocator_concept* ac);

/**
 * @brief Destroys the queue ADT.
 * @param[in, out] lq Pointer to the queue instance.
 * @param[in] deinit Pointer to func pointer used to deinit objects.
 * Might be NULL.
 * @see object_concept
 */
void lqueue_destroy(struct lqueue *lq, deinit_cb deinit);

/** @return Size of the node in bytes allocated by allocator_concept */
size_t lqueue_node_sizeof();

/** @} */ // End of Create & Destroy

/**
 * @name Enqueue & Dequeue
 * Functions to enqueue and dequeue items.
 * @{
 */

/**
 * @brief Enqueues new item.
 * @param[in] new_item new item to be enqueued into the queue.
 * @return 0 on success, non-zero otherwise.
 */
int lenqueue(struct lqueue *lq, void *new_item);

/**
 * @brief Dequeues the stack ADT.
 * @return Reference to an object, supplied by user,
 * stored in the queue
 */
void *ldequeue(struct lqueue *lq);

/** @} */ // End of Enqueue & Dequeue

/**
 * @name Inspection
 * Functions to query stack ADT.
 * @{
 */

/** @return Reference to the data stored at the front. */
void *lqueue_front(struct lqueue *lq);

/** @return Reference to the data stored at the rear. */
void *lqueue_rear(struct lqueue *lq);

/** @return 1 if the queue is empty, 0 otherwise. */
int lqueue_empty(const struct lqueue *lq);

/** @return Count of the objects stored in the queue. */
size_t lqueue_size(const struct lqueue *lq);

/** @} */ // End of Inspection

/**
 * @brief Iterates over the queue.
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void lqueue_walk(struct lqueue *lq, void *context, void (*handler) (void *item, void *context));

/** @} */ // End of LQUEUE group

#ifdef __cplusplus
}
#endif

#endif // QUEUE_LQUEUE_H