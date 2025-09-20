#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

#define size_st(st) ((st)->top)
#define is_st_empty(st) (((st)->top == 0))

struct stack
{
    char* contents;
    size_t capacity;
    size_t top;
    size_t obj_size;
};

// init stack, returns 0 if it succeeds, 1 otherwise
int stack_init(struct stack* st, size_t obj_size);
// push an item, returns 0 if it succeeds, 1 otherwise
int push(struct stack* st, const void* new);
// pop an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int pop(struct stack* st, void* result);
// peek an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int stack_peek(const struct stack* st, void* result);
// walk stack by given handler defined by the user according to item type they enqueue
void stack_walk(struct stack* st, void* userdata, void (*handler) (void* item, void* userdata));
// free stack contents (freeing stack itself, if dynamically allocated, is on you)
// if you dont store pointers to dynamic allocated objects, this function with a deallocator might be dangerous !!!
void stack_free(struct stack* st, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // STACK_H