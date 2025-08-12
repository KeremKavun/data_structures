#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdbool.h>

#define size_st(st) ((st)->top)
#define is_st_empty(st) (((st)->top == 0))

struct Stack
{
    size_t capacity;
    size_t top;
    size_t obj_size;
    void** contents;
};

// init stack, returns either EXIT_SUCCESS OR EXIT_FAILURE
int init_st(struct Stack* st, size_t _obj_size);
// push an item, returns either EXIT_SUCCESS OR EXIT_FAILURE
int push(struct Stack* st, const void* _new);
// pop an item, returns either item or NULL, you should free item since ownership is yours
void* pop(struct Stack* st);
// peek an item, returns either item or NULL
void* peek_st(const struct Stack* st);
// free stack contents (freeing stack itself, if dynamically allocated, is on you)
void free_st(struct Stack* st);

#endif // STACK_H