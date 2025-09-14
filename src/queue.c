#include "../include/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int init_q(struct Queue* q, size_t _obj_size)
{
    q->capacity = 2;
    q->size =0;
    q->front = 0;
    q->rear = 0;
    q->obj_size = _obj_size;
    q->contents = malloc(sizeof(void*) * q->capacity);
    if (!q->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    return 0;
}

int enqueue(struct Queue* q, const void* _new)
{
    if (q->capacity == size_q(q))
    {
        void** new_contents = malloc(sizeof(void*) * q->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        
        for (size_t i = 0; i < size_q(q); ++i)
            new_contents[i] = q->contents[(q->front + i) % q->capacity];
        
        free(q->contents);
        q->capacity *= 2;
        q->front = 0;
        q->rear = q->size;
        q->contents = new_contents;
    }
    q->contents[q->rear] = malloc(q->obj_size);
    if (!q->contents[q->rear])
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    memcpy(q->contents[q->rear], _new, q->obj_size);
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return 0;
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
        return NULL;
    return q->contents[q->front];
}

void print_q(const struct Queue* q, void (*handler) (void* item))
{
    for (size_t i = q->front; i != q->rear; i = (i + 1) % q->capacity)
        handler(q->contents[i]);
}

void free_q(struct Queue* q)
{
    if (q->contents)
    {
        for (size_t i = q->front; i != q->rear; i = (i + 1) % q->capacity)
            free(q->contents[i]);
        free(q->contents);
        q->contents = NULL;
        q->capacity = q->front = q->rear = q->size = 0;
    }
}