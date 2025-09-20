#include "../include/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// queue implementation with fixed size types and internal char* buffer (no dynamic allocation and void**)                                  //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static size_t char_index(size_t ind, size_t obj_size);

int stack_init(struct stack* st, size_t obj_size)
{
    st->contents = malloc(sizeof(char) * obj_size * INITIAL_CAPACITY);
    if (!st->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    st->capacity = INITIAL_CAPACITY;
    st->top= 0;
    st->obj_size = obj_size;
    return 0;
}

int push(struct stack* st, const void* new)
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

int pop(struct stack* st, void* result)
{
    if (is_st_empty(st))
        return 1;
    st->top--; 
    memcpy(result, &st->contents[char_index(st->top, st->obj_size)], st->obj_size);
    return 0;
}

int stack_peek(const struct stack* st, void* result)
{
    if (is_st_empty(st))
        return 1;
    memcpy(result, &st->contents[char_index(st->top - 1, st->obj_size)], st->obj_size);
    return 0;
}

void stack_walk(struct stack* st, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = 0; i < size_st(st); i++)
        handler((void*) &st->contents[char_index(i, st->obj_size)], userdata);
}

void stack_free(struct stack* st, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        stack_walk(st, userdata, deallocator);
    free(st->contents);
    st->contents = NULL;
    st->capacity = 2; st->top= 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}