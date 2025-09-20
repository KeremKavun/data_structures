#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

// replace with 'size' attribute of given struct queue*
#define size_sq(sq) ((sq)->size)
// replace with bool indicating whether given struct queue* is empty or not
#define is_sq_empty(sq) (((sq)->size == 0))

// struct queue, 48 bytes in stack plus dynamically allocated 'contents' array at the heap
struct safe_queue
{
    void** contents;
    size_t capacity;
    size_t size;
    size_t front;
    size_t rear;
};

// init queue, returns 0 if it succeeds, 1 otherwise
int safe_queue_init(struct safe_queue* sq);
// enqueue an item, returns 0 if it succeeds, 1 otherwise
int safe_enqueue(struct safe_queue* sq, const void* new, size_t obj_size);
// dequeue an item, returns either item or NULL
void* safe_dequeue(struct safe_queue* sq);
// peek an item, returns either item or NULL
void* safe_queue_peek(const struct safe_queue* sq);
// walk queue by given handler defined by the user according to item type they enqueue
void safe_queue_walk(const struct safe_queue* sq, void* userdata, void (*handler) (void* item, void* userdata));
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
void safe_queue_free(struct safe_queue* sq, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // SAFE_QUEUE_H