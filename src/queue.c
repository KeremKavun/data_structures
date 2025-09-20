#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// queue implementation with fixed size types and internal char* buffer (no dynamic allocation and void**)                                  //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static size_t char_index(size_t ind, size_t obj_size);

int queue_init(struct queue* q, size_t obj_size)
{
    q->contents = malloc(sizeof(char) * obj_size * INITIAL_CAPACITY);
    if (!q->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    q->capacity = INITIAL_CAPACITY;
    q->size =0;
    q->front = 0;
    q->rear = 0;
    q->obj_size = obj_size;
    return 0;
}

int enqueue(struct queue* q, const void* new)
{
    if (q->capacity == size_q(q))
    {
        char* new_contents = malloc(sizeof(char) * q->obj_size * q->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        
        for (size_t i = 0; i < size_q(q); ++i)
            memcpy(&new_contents[char_index(i, q->obj_size)], &q->contents[char_index((q->front + i) % q->capacity, q->obj_size)], q->obj_size);
        
        free(q->contents);
        q->contents = new_contents;
        q->capacity *= 2;
        q->front = 0;
        q->rear = q->size;
    }
    memcpy((void*) &q->contents[char_index(q->rear, q->obj_size)], new, q->obj_size);
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return 0;
}

int dequeue(struct queue* q, void* result)
{
    if (is_q_empty(q))
        return 1;
    memcpy(result, &q->contents[char_index(q->front, q->obj_size)], q->obj_size);
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return 0;
}

int queue_peek(const struct queue* q, void* result)
{
    if (is_q_empty(q))
        return 1;
    memcpy(result, &q->contents[char_index(q->front, q->obj_size)], q->obj_size);
    return 0;
}

void queue_walk(const struct queue* q, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = q->front; i != q->rear; i = (i + 1) % q->capacity)
        handler((void*) &q->contents[char_index(i, q->obj_size)], userdata);
}

void queue_free(struct queue* q, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        queue_walk(q, userdata, deallocator);
    free(q->contents);
    q->contents = NULL;
    q->capacity = q->front = q->rear = q->size = 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}