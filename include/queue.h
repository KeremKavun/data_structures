#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdbool.h>

// replace with 'size' attribute of given struct Queue*
#define size_q(q) ((q)->size)
// replace with bool indicating whether given struct Queue* is empty or not
#define is_q_empty(q) (((q)->size == 0))

// struct Queue, 48 bytes in stack plus dynamically allocated 'contents' array at the heap
struct Queue
{
    size_t capacity;
    size_t size;
    size_t front;
    size_t rear;
    size_t obj_size;
    void** contents;
};

// init queue, returns either EXIT_SUCCESS OR EXIT_FAILURE
int init_q(struct Queue* q, size_t _obj_size);
// enqueue an item, returns either EXIT_SUCCESS OR EXIT_FAILURE
int enqueue(struct Queue* q, const void* _new);
// dequeue an item, returns either item or NULL, you should free item since ownership is yours
void* dequeue(struct Queue* q);
// peek an item, returns either item or NULL
void* peek_q(const struct Queue* q);
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
void free_q(struct Queue* q);

#endif // QUEUE_H