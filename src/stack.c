#include "../include/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static size_t char_index(size_t ind, size_t obj_size);

int init_st(struct Stack* st, size_t obj_size)
{
    st->contents = malloc(sizeof(char) * obj_size * st->capacity);
    if (!st->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    st->capacity = 2;
    st->top= 0;
    st->obj_size = obj_size;
    return 0;
}

int push(struct Stack* st, const void* new)
{
    if (st->capacity == size_st(st))
    {
        char* new_contents = realloc(st->contents, sizeof(char) * st->obj_size * st->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        st->contents = new_contents;
        st->capacity *= 2;
    }
    memcpy((void*) &st->contents[char_index(st->top, st->obj_size)], new, st->obj_size);
    st->top++;
    return 0;
}

void* pop(struct Stack* st)
{
    if (is_st_empty(st))
        return NULL;
    st->top--; 
    void* item = &st->contents[char_index(st->top, st->obj_size)];
    return item;
}

void* peek_st(const struct Stack* st)
{
    if (is_st_empty(st))
        return NULL;
    return (void*) &st->contents[char_index(st->top - 1, st->obj_size)];
}

void walk_st(const struct Stack* st, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = 0; i < size_st(st); i++)
        handler((void*) &st->contents[char_index(i, st->obj_size)], userdata);
}

void free_st(struct Stack* st, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        walk_st(st, userdata, deallocator);
    free(st->contents);
    st->contents = NULL;
    st->capacity = 2; st->top= 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}