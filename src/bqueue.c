#include "../include/bqueue.h"
#include "../../buffers/include/cbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int bqueue_init(struct bqueue* bq, size_t obj_size)
{
    struct cbuffer* contents = malloc(sizeof(struct cbuffer));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    if (cbuffer_init(contents, INITIAL_CAPACITY, obj_size) != 0)
    {
        LOG(LIB_LVL, CERROR, "COuld not initialize cbuffer");
        return 1;
    }
    bq->contents = contents;
    return 0;
}

void bqueue_deinit(struct bqueue* bq, void* context, struct object_concept* oc)
{
    cbuffer_deinit(bq->contents, context, oc);
    free(bq->contents);
    bq->contents = NULL;
}

/*───────────────────────────────────────────────
 * Enqueue & Dequeue
 *───────────────────────────────────────────────*/

int benqueue(struct bqueue* bq, const void* new_item)
{
    LOG(LIB_LVL, CINFO, "Enqueuing item at address %p, by copying", new_item);
    return cbuffer_push_back(bq->contents, new_item);
}

int emplace_benqueue(struct bqueue* bq, void* args, struct object_concept* oc)
{
    LOG(LIB_LVL, CINFO, "Enqueuing item by emplacing");
    return cbuffer_emplace_back(bq->contents, args, oc);
}

int bdequeue(struct bqueue* bq, void* result)
{
    LOG(LIB_LVL, CINFO, "Dequeuing item");
    return cbuffer_pop_front(bq->contents, result);
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bqueue_front(const struct bqueue* bq, void* result)
{
    return cbuffer_get(bq->contents, 0, result);
}

int bqueue_rear(const struct bqueue* bq, void* result)
{
    return cbuffer_get(bq->contents, bqueue_size(bq) - 1, result);
}

int bqueue_empty(const struct bqueue* bq)
{
    return cbuffer_empty(bq->contents);
}

size_t bqueue_size(const struct bqueue* bq)
{
    return cbuffer_size(bq->contents);
}

size_t bqueue_capacity(const struct bqueue* bq)
{
    return cbuffer_capacity(bq->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bqueue_walk(const struct bqueue* bq, void* context, void (*handler) (void* item, void* context))
{
    size_t size = bqueue_size(bq);
    for (size_t i = 0; i < size; ++i)
        handler(cbuffer_at(bq->contents, i), context);
}