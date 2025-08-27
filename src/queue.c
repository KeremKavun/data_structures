#include "../include/queue.h"
#include "../../utils/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG
#include "../../debug/include/debug.h"

int init_q(struct Queue* q, size_t _obj_size)
{
    q->capacity = 2;
    q->size =0;
    q->front = 0;
    q->rear = 0;
    q->obj_size = _obj_size;
    q->contents = malloc(sizeof(void*) * q->capacity);
    if (!q->contents)
        RETURN(LOG("Allocation failure"), EXIT_FAILURE);
    else
        RETURN(LOG("%s succeeded", __func__), EXIT_SUCCESS);
}

int enqueue(struct Queue* q, const void* _new)
{
    if (q->capacity == size_q(q))
    {
        void** new_contents = malloc(sizeof(void*) * q->capacity * 2);
        if (!new_contents)
            RETURN(LOG("Allocation failure"), EXIT_FAILURE);
        
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
        RETURN(LOG("Allocation failure"), EXIT_FAILURE);
    memcpy(q->contents[q->rear], _new, q->obj_size);
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    RETURN(LOG("%s succeeded", __func__), EXIT_SUCCESS);
}

void* dequeue(struct Queue* q)
{
    if (is_q_empty(q))
        RETURN(NULL);
    void* item = q->contents[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    RETURN(LOG("%s succeeded", __func__), item); 
}

void* peek_q(const struct Queue* q)
{
    if (is_q_empty(q))
        RETURN(LOG("Queue underflow"), NULL);
    RETURN(LOG("%s succeeded", __func__), q->contents[q->front]);
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
        LOG("%s succeeded", __func__);
    }
}