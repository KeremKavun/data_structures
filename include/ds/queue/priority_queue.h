#ifndef QUEUE_PRIORITY_QUEUE_H
#define QUEUE_PRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ds/utils/object_concept.h>
#include <ds/utils/debug.h>
#include <stddef.h>

/**
 * @file priority_queue.h
 * @brief Defines the interface for priority queue.
 */

/**
 * @defgroup PRIORITYQUEUE Priority Queue (Heap-based)
 * @ingroup QUEUE
 * @brief A Priority Queue implementation wrapping an `array_heap`.
 * 
 * @details
 * Literally just a wrapper of heap. But this might be changed, since this
 * is an ADT.
 * Elements are dequeued based on the result of the comparison function 
 * rather than insertion order.
 * @{
 */
struct priority_queue;

/**
 * @name Lifecycle
 * @{
 */

/**
 * @brief Creates a new Priority Queue.
 * @param[in] obj_size Size of the elements to be stored.
 * @param[in] oc Object concept for management.
 * @param[in] cmp Comparison function. Returns >0 if a has higher priority than b.
 * @return Pointer to new priority_queue, or NULL on failure.
 */
struct priority_queue *priority_queue_create(size_t obj_size, struct object_concept *oc, int (*cmp) (const void *a, const void *b));

void priority_queue_destroy(struct priority_queue* pq);

/** @} */

/**
 * @name Operations
 * @{
 */

/**
 * @brief Inserts an item based on priority.
 * @param[in,out] pq Priority Queue instance.
 * @param[in] item Pointer to data to copy into the queue.
 * @return 0 on success, non-zero on overflow/failure.
 */
int priority_queue_enqueue(struct priority_queue* pq, void* item);

/**
 * @brief Removes the highest priority item.
 * @param[in,out] pq Priority Queue instance.
 * @param[out] removed Buffer to copy the removed data into.
 * @return 0 on success, non-zero if empty.
 */
int priority_queue_dequeue(struct priority_queue* pq, void *removed);

/** @} */

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const void* priority_queue_front(const struct priority_queue* pq);
const void* priority_queue_rear(const struct priority_queue* pq);
int priority_queue_empty(const struct priority_queue* pq);
size_t priority_queue_size(const struct priority_queue* pq);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void priority_queue_walk(struct priority_queue* pq, void* context, void (*handler) (void* item, void* context));

/** @} */ // End of the PRIORITYQUEUE group

#ifdef __cplusplus
}
#endif

#endif // QUEUE_PRIORITY_QUEUE_H