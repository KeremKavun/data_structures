#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static size_t index(size_t ind, size_t obj_size);

int init_q(struct queue* q, size_t obj_size)
{
    q->contents = malloc(sizeof(char) * obj_size * q->capacity);
    if (!q->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    q->capacity = 2;
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
        void** new_contents = malloc(sizeof(char) * q->obj_size* q->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        
        for (size_t i = 0; i < size_q(q); ++i)
            new_contents[index(i, q->obj_size)] = q->contents[index((q->front + i) % q->capacity, q->obj_size)];
        
        free(q->contents);
        q->contents = new_contents;
        q->capacity *= 2;
        q->front = 0;
        q->rear = q->size;
    }
    memcpy(q->contents[index(q->rear, q->obj_size)], new, q->obj_size);
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return 0;
}

void* dequeue(struct queue* q)
{
    if (is_q_empty(q))
        return NULL;
    void* item = q->contents[index(q->front, q->obj_size)];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return item;
}

void* peek_q(const struct queue* q)
{
    if (is_q_empty(q))
        return NULL;
    return q->contents[index(q->front, q->obj_size)];
}

void walk_q(const struct queue* q, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = q->front; i != q->rear; i = (i + 1) % q->capacity)
        handler(q->contents[index(i, q->obj_size)], userdata);
}

void free_q(struct queue* q, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        walk_q(q, userdata, deallocator);
    free(q->contents);
    q->contents = NULL;
    q->capacity = q->front = q->rear = q->size = 0;
}

// *** Helper functions *** //

static inline size_t index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}