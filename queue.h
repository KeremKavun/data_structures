#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdbool.h>

#define size_q(q) ((q)->size)
#define is_q_empty(q) (((q)->size == 0))

struct Queue
{
    size_t capacity;
    size_t size;
    size_t front;
    size_t rear;
    void** contents;
};

void init_q(struct Queue* q);
struct Queue* malloc_q();
void enqueue(struct Queue* q, void* _new);
void* dequeue(struct Queue* q);
void* peek_q(const struct Queue* q);
void free_q(struct Queue* q);

#endif // QUEUE_H