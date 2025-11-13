#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

// struct queue, 48 bytes in stack plus dynamically allocated 'contents' array at the heap
struct bqueue
{
    struct cbuffer* contents;
};

typedef struct bqueue bqueue_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// init queue, returns 0 if it succeeds, 1 otherwise
int bqueue_init(struct bqueue* bq, size_t obj_size);
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
// if you dont store pointers to dynamic allocated objects, this function with a deallocator might be dangerous !!!
void bqueue_deinit(struct bqueue* bq, void* userdata, void (*deallocator) (void* item, void* userdata));

/*───────────────────────────────────────────────
 * Enqueue & Dequeue
 *───────────────────────────────────────────────*/

// enqueue an item by copying, returns 0 if it succeeds, 1 otherwise
int benqueue(struct bqueue* bq, const void* new_item, void* userdata, int (*copy) (const void* new_item, void* queue_item, void* userdata));
// enqueue an item by initializing the object in place, queue will own the object 
// must be careful if the given object stores pointers to the objects in the heap, you must provide a deallocator in this case
int emplace_benqueue(struct bqueue* bq, void* userdata, int (*init) (void* item, void* userdata));
// dequeue an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int bdequeue(struct bqueue* bq, void* result);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

// Get the front of the queue
int bqueue_front(const struct bqueue* bq, void* result);
// Get the rear of the queue
int bqueue_rear(const struct bqueue* bq, void* result);
// Returns 1 if the bqueue is empty, 0 otherwise
int bqueue_empty(const struct bqueue* bq);
// Returns the size of the bqueue
size_t bqueue_size(const struct bqueue* bq);
// Returns the total number of objects with obj_size passed to the initializer
size_t bqueue_capacity(const struct bqueue* bq);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

// walk queue by given handler defined by the user according to item type they enqueue
void bqueue_walk(const struct bqueue* bq, void* userdata, void (*handler) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // QUEUE_H