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
    void** contents;
};

void init_st(struct Stack* st);
struct Stack* malloc_st();
void push(struct Stack* st, void* _new);
void* pop(struct Stack* st);
void* peek_st(const struct Stack* st);
void free_st(struct Stack* st);

#endif // STACK_H