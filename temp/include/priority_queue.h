#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

struct priority_queue;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct priority_queue *priority_queue_create(size_t obj_size, struct object_concept *oc, int (*cmp) (const void *a, const void *b));
void priority_queue_destroy(struct priority_queue* pq);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int priority_queue_enqueue(struct priority_queue* pq, void* item);
int priority_queue_dequeue(struct priority_queue* pq,void *removed);

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

#ifdef __cplusplus
}
#endif

#endif // PRIORITY_QUEUE_H