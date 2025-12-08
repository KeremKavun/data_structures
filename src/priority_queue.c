#include "../include/priority_queue.h"
#include "../../trees/src/heap.c"
#include "../../trees/internals/heap_definition.h"
#include <stdlib.h>

struct priority_queue
{
    struct heap heap;
};

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int priority_queue_init(struct priority_queue* pq, int (*cmp) (const void* a, const void* b))
{
    if (heap_init(&pq->heap, 1, cmp) != 0)
    {
        LOG(LIB_LVL, CERROR, "Failed to create heap for priority queue");
        return 1;
    }
    return 0;
}

void priority_queue_deinit(struct priority_queue* pq, void* context, struct object_concept* oc)
{
    heap_deinit(&pq->heap, context, oc);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int priority_queue_enqueue(struct priority_queue* pq, void* item)
{
    return heap_add(&pq->heap, item);
}

void* priority_queue_dequeue(struct priority_queue* pq)
{
    return heap_remove(&pq->heap);
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const void* priority_queue_front(const struct priority_queue* pq)
{
    return *(void**)lbuffer_at((struct lbuffer*) &pq->heap.buffer, 0);
}

const void* priority_queue_rear(const struct priority_queue* pq)
{
    return *(void**)lbuffer_at((struct lbuffer*) &pq->heap.buffer, lbuffer_size((struct lbuffer*) &pq->heap.buffer) - 1);
}

int priority_queue_empty(const struct priority_queue* pq)
{
    return heap_empty(&pq->heap);
}

size_t priority_queue_size(const struct priority_queue* pq)
{
    return heap_size(&pq->heap);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void priority_queue_walk(struct priority_queue* pq, void* userdata, void (*handler) (void* data, void* userdata))
{
    heap_walk(&pq->heap, userdata, handler);
}
