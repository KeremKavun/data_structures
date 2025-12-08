#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

struct priority_queue;
typedef struct priority_queue priority_queue_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int priority_queue_init(struct priority_queue* pq, int (*cmp) (const void* a, const void* b));
void priority_queue_deinit(struct priority_queue* pq, void* context, struct object_concept* oc);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int priority_queue_enqueue(struct priority_queue* pq, void* item);
void* priority_queue_dequeue(struct priority_queue* pq);

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