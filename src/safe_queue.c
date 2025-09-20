#include "../include/safe_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// safe_queue implementation with multiple types, dynamic allocations and void**                                                             //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static void default_deallocator(void* item, void* userdata);

int safe_queue_init(struct safe_queue* sq)
{
    sq->contents = malloc(sizeof(void*) * INITIAL_CAPACITY);
    if (!sq->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    sq->capacity = INITIAL_CAPACITY;
    sq->size =0;
    sq->front = 0;
    sq->rear = 0;
    return 0;
}

int safe_enqueue(struct safe_queue* sq, const void* new, size_t obj_size)
{
    if (sq->capacity == size_sq(sq))
    {
        void** new_contents = malloc(sizeof(void*) * sq->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        
        for (size_t i = 0; i < size_sq(sq); ++i)
            new_contents[i] = sq->contents[(sq->front + i) % sq->capacity];
        
        free(sq->contents);
        sq->contents = new_contents;
        sq->capacity *= 2;
        sq->front = 0;
        sq->rear = sq->size;
    }
    sq->contents[sq->rear] = malloc(obj_size);
    if (!sq->contents[sq->rear])
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    memcpy(sq->contents[sq->rear], new, obj_size);
    sq->rear = (sq->rear + 1) % sq->capacity;
    sq->size++;
    return 0;
}

void* safe_dequeue(struct safe_queue* sq)
{
    if (is_sq_empty(sq))
        return NULL;
    void* item = sq->contents[sq->front];
    sq->front = (sq->front + 1) % sq->capacity;
    sq->size--;
    return item;
}

void* safe_queue_peek(const struct safe_queue* sq)
{
    if (is_sq_empty(sq))
        return NULL;
    return sq->contents[sq->front];
}

void safe_queue_walk(const struct safe_queue* sq, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = sq->front; i != sq->rear; i = (i + 1) % sq->capacity)
        handler(sq->contents[i], userdata);
}

void safe_queue_free(struct safe_queue* sq, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    safe_queue_walk(sq, userdata, (deallocator) ? deallocator : default_deallocator);
    free(sq->contents);
    sq->contents = NULL;
    sq->capacity = sq->front = sq->rear = sq->size = 0;
}

// *** Helper functions *** //

static void default_deallocator(void* item, void* userdata)
{
    free(item);
}