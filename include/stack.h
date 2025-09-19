#ifndef STACK_H
#define STACK_H

#include "../../debug/include/debug.h"
#include <stddef.h>
#include <stdbool.h>

#define size_st(st) ((st)->top)
#define is_st_empty(st) (((st)->top == 0))

struct Stack
{
    char* contents;
    size_t capacity;
    size_t top;
    size_t obj_size;
};

// init stack, returns 0 if it succeeds, 1 otherwise
int init_st(struct Stack* st, size_t obj_size);
// push an item, returns 0 if it succeeds, 1 otherwise
int push(struct Stack* st, const void* new);
// pop an item, returns either item or NULL
void* pop(struct Stack* st);
// peek an item, returns either item or NULL
void* peek_st(const struct Stack* st);
// walk stack by given handler defined by the user according to item type they enqueue
void walk_st(const struct Stack* st, void* userdata, void (*handler) (void* item, void* userdata));
// free stack contents (freeing stack itself, if dynamically allocated, is on you)
void free_st(struct Stack* st, void* userdata, void (*deallocator) (void* item, void* userdata));

#endif // STACK_H