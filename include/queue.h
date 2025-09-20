#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

// replace with 'size' attribute of given struct queue*
#define size_q(q) ((q)->size)
// replace with bool indicating whether given struct queue* is empty or not
#define is_q_empty(q) (((q)->size == 0))

// struct queue, 48 bytes in stack plus dynamically allocated 'contents' array at the heap
struct queue
{
    char* contents;
    size_t capacity;
    size_t size;
    size_t front;
    size_t rear;
    size_t obj_size;
};

// init queue, returns 0 if it succeeds, 1 otherwise
int queue_init(struct queue* q, size_t obj_size);
// enqueue an item, returns 0 if it succeeds, 1 otherwise
int enqueue(struct queue* q, const void* new);
// dequeue an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int dequeue(struct queue* q, void* result);
// peek an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int queue_peek(const struct queue* q, void* result);
// walk queue by given handler defined by the user according to item type they enqueue
void queue_walk(const struct queue* q, void* userdata, void (*handler) (void* item, void* userdata));
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
// if you dont store pointers to dynamic allocated objects, this function with a deallocator might be dangerous !!!
void queue_free(struct queue* q, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // QUEUE_H