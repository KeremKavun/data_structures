#ifndef QUEUE_H
#define QUEUE_H

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

// init queue, returns either EXIT_SUCCESS OR EXIT_FAILURE
int init_q(struct queue* q, size_t obj_size);
// enqueue an item, returns either EXIT_SUCCESS OR EXIT_FAILURE
int enqueue(struct queue* q, const void* new);
// dequeue an item, returns either item or NULL, you should free item since ownership is yours
void* dequeue(struct queue* q);
// peek an item, returns either item or NULL
void* peek_q(const struct queue* q);
// print queue by given handler defined by the user according to item type they enqueue
void walk_q(const struct queue* q, void* userdata, void (*handler) (void* item, void* userdata));
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
void free_q(struct queue* q, void* userdata, void (*deallocator) (void* item, void* userdata));

#endif // QUEUE_H