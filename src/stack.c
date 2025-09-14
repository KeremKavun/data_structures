#include "../include/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int init_st(struct Stack* st, size_t _obj_size)
{
    st->capacity = 2;
    st->top= 0;
    st->obj_size = _obj_size;
    st->contents = malloc(sizeof(void*) * st->capacity);
    if (!st->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    return 0;
}

int push(struct Stack* st, const void* _new)
{
    if (st->capacity == size_st(st))
    {
        void** new_contents = realloc(st->contents, sizeof(void*) * st->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        st->contents = new_contents;
        st->capacity *= 2;
    }
    st->contents[st->top] = malloc(st->obj_size);
    if (!st->contents[st->top])
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    memcpy(st->contents[st->top], _new, st->obj_size);
    st->top++;
    return 0;
}

void* pop(struct Stack* st)
{
    if (is_st_empty(st))
        return NULL;
    st->top--; 
    void* item = st->contents[st->top];
    return item;
}

void* peek_st(const struct Stack* st)
{
    if (is_st_empty(st))
        return NULL;
    return st->contents[st->top - 1];
}

void free_st(struct Stack* st)
{
    for (size_t i = 0; i < size_st(st); i++)
        free(st->contents[i]);
    free(st->contents);
    st->contents = NULL;
    st->capacity = 2;
    st->top= 0;
}

