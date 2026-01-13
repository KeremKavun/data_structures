#include <ds/queue/priority_queue.h>
#include <ds/trees/array_heap.h>
#include <ds/arrays/dynarray.h>
#include <stdlib.h>

struct priority_queue
{
    struct array_heap heap;
};

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct priority_queue *priority_queue_create(size_t obj_size, struct object_concept *oc, int (*cmp) (const void *a, const void *b))
{
    struct priority_queue *pq = malloc(sizeof(struct priority_queue));
    if (!pq) {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return NULL;
    }
    if (array_heap_init(&pq->heap, obj_size, oc, cmp) != 0) {
        LOG(LIB_LVL, CERROR, "Failed to create heap for priority queue");
        free(pq);
        return NULL;
    }
    return pq;
}

void priority_queue_destroy(struct priority_queue* pq)
{
    array_heap_deinit(&pq->heap);
    free(pq);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int priority_queue_enqueue(struct priority_queue* pq, void* item)
{
    return array_heap_add(&pq->heap, item);
}

int priority_queue_dequeue(struct priority_queue* pq, void *removed)
{
    return array_heap_remove(&pq->heap, removed);
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const void* priority_queue_front(const struct priority_queue* pq)
{
    return dynarray_front((struct dynarray*) &pq->heap.contents);
}

const void* priority_queue_rear(const struct priority_queue* pq)
{
    return dynarray_back((struct dynarray*) &pq->heap.contents);
}

int priority_queue_empty(const struct priority_queue* pq)
{
    return array_heap_empty(&pq->heap);
}

size_t priority_queue_size(const struct priority_queue* pq)
{
    return array_heap_size(&pq->heap);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void priority_queue_walk(struct priority_queue* pq, void* context, void (*handler) (void* item, void* context))
{
    array_heap_walk(&pq->heap, context, handler);
}
