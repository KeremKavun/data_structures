#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#define DEBUG
#include "../debug/debug.h"

void init_st(struct Stack* st)
{
    st->capacity = 2;
    st->top= 0;
    st->contents = malloc(sizeof(void*) * st->capacity);
    if (!st->contents)
        LOG("Allocation failure at function %s", __func__);
}

struct Stack* malloc_st()
{
    struct Stack* st = malloc(sizeof(struct Stack));
    if (!st)
    {
        LOG("Allocation failure at function %s", __func__);
        return NULL;
    }
    init_st(st);
    return st;
}

void push(struct Stack* st, void* _new)
{
    if (st->capacity == size_st(st))
    {
        void** new_contents = realloc(st->contents, sizeof(void*) * st->capacity * 2);
        if (!new_contents)
        {
            LOG("Allocation failure at function %s", __func__);
            return;
        }
        st->contents = new_contents;
        st->capacity *= 2;
    }

    st->contents[st->top++] = _new;
}

void* pop(struct Stack* st)
{
    if (is_st_empty(st))
    {
        LOG("Stack underflow at function %s", __func__);
        return NULL;
    }

    return st->contents[--st->top];
}

void* peek_st(const struct Stack* st)
{
    if (is_st_empty(st))
    {
        LOG("Stack underflow at function %s", __func__);
        return NULL;
    }

    return st->contents[st->top - 1];
}

void free_st(struct Stack* st)
{
    free(st->contents);
    free(st);
}

