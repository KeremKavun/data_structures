#include "../include/bqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

static size_t char_index(size_t ind, size_t obj_size);
static int queue_realloc(struct bqueue* bq);

int bqueue_init(struct bqueue* bq, size_t obj_size)
{
    bq->contents = malloc(sizeof(char) * obj_size * INITIAL_CAPACITY);
    if (!bq->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    bq->capacity = INITIAL_CAPACITY;
    bq->size =0;
    bq->front = 0;
    bq->rear = 0;
    bq->obj_size = obj_size;
    return 0;
}

int benqueue(struct bqueue* bq, const void* new_item, void* userdata, int (*copy) (const void* new_item, void* queue_item, void* userdata))
{
    if (queue_realloc(bq) != 0)
    {
        LOG(LIB_LVL, CERROR, "queue_realloc failed");
        return 1;
    }
    if (copy(new_item, (void*) &bq->contents[char_index(bq->rear, bq->obj_size)], userdata) != 0)
    {
        LOG(PROJ_LVL, CERROR, "Copy ctor failed");
        return 1;
    }
    bq->rear = (bq->rear + 1) % bqueue_capacity(bq);
    bq->size++;
    return 0;
}

int emplace_benqueue(struct bqueue* bq, void* userdata, int (*init) (void* item, void* userdata))
{
    if (queue_realloc(bq) != 0)
    {
        LOG(LIB_LVL, CERROR, "queue_realloc failed");
        return 1;
    }
    if (init((void*) &bq->contents[char_index(bq->rear, bq->obj_size)], userdata) != 0)
    {
        LOG(PROJ_LVL, CERROR, "Initializer failed");
        return 1;
    }
    bq->rear = (bq->rear + 1) % bqueue_capacity(bq);
    bq->size++;
    return 0;
}

int bdequeue(struct bqueue* bq, void* result)
{
    if (bqueue_empty(bq))
        return 1;
    if (result)
        memcpy(result, &bq->contents[char_index(bq->front, bq->obj_size)], bq->obj_size);
    bq->front = (bq->front + 1) % bqueue_capacity(bq);
    bq->size--;
    return 0;
}

int bqueue_peek(const struct bqueue* bq, void* result)
{
    if (bqueue_empty(bq))
        return 1;
    memcpy(result, &bq->contents[char_index(bq->front, bq->obj_size)], bq->obj_size);
    return 0;
}

int bqueue_empty(const struct bqueue* bq)
{
    return bq->size == 0;
}

size_t bqueue_size(const struct bqueue* bq)
{
    return bq->size;
}

size_t bqueue_capacity(const struct bqueue* bq)
{
    return bq->capacity;
}

void bqueue_walk(const struct bqueue* bq, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = bq->front; i != bq->rear; i = (i + 1) % bqueue_capacity(bq))
        handler((void*) &bq->contents[char_index(i, bq->obj_size)], userdata);
}

void bqueue_free(struct bqueue* bq, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        bqueue_walk(bq, userdata, deallocator);
    free(bq->contents);
    bq->contents = NULL;
    bq->capacity = bq->front = bq->rear = bq->size = 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}

static int queue_realloc(struct bqueue* bq)
{
    if (bqueue_capacity(bq) == bqueue_size(bq))
    {
        char* new_contents = malloc(sizeof(char) * bq->obj_size * bqueue_capacity(bq) * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        
        for (size_t i = 0; i < bqueue_size(bq); ++i)
            memcpy(&new_contents[char_index(i, bq->obj_size)], &bq->contents[char_index((bq->front + i) % bqueue_capacity(bq), bq->obj_size)], bq->obj_size);
        
        free(bq->contents);
        bq->contents = new_contents;
        bq->capacity *= 2;
        bq->front = 0;
        bq->rear = bq->size;
    }
    return 0;
}