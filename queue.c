#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define DEBUG
#include "../debug/debug.h"

void init_q(struct Queue* q)
{
    q->capacity = 2;
    q->size =0;
    q->front = 0;
    q->rear = 0;
    q->contents = malloc(sizeof(void*) * q->capacity);
    if (!q->contents)
        LOG("Allocation failure at function %s", __func__);
}

struct Queue* malloc_q()
{
    struct Queue* q = malloc(sizeof(struct Queue));
    if (!q)
    {
        LOG("Allocation failure at function %s", __func__);
        return NULL;
    }
    init_q(q);
    return q;
}

void enqueue(struct Queue* q, void* _new)
{
    if (q->capacity == size_q(q))
    {
        void** new_contents = malloc(sizeof(void*) * q->capacity * 2);
        if (!new_contents)
        {
            LOG("Allocation failure at function %s", __func__);
            return;
        }
        
        for (size_t i = 0; i < size_q(q); ++i)
            new_contents[i] = q->contents[(q->front + i) % q->capacity];
        
        free(q->contents);
        q->capacity *= 2;
        q->front = 0;
        q->rear = q->size;
        q->contents = new_contents;
    }
    q->contents[q->rear] = _new;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

void* dequeue(struct Queue* q)
{
    if (is_q_empty(q))
        return NULL;
    void* item = q->contents[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return item;
}


void* peek_q(const struct Queue* q)
{
    if (is_q_empty(q))
    {
        LOG("Queue underflow at function %s", __func__);
        return NULL;
    }
    return q->contents[q->front];
}

void free_q(struct Queue* q)
{
    free(q->contents);
    free(q);
}