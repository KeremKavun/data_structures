#include "../include/bstack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// queue implementation with fixed size types and internal char* buffer (no dynamic allocation and void**)                                  //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static size_t char_index(size_t ind, size_t obj_size);
static int stack_realloc(struct bstack* st);

int bstack_init(struct bstack* st, size_t obj_size)
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

int bpush(struct bstack* st, const void* new_item, void (*copy) (const void* new_item, void* queue_item))
{
    if (stack_realloc(st) != 0)
    {
        LOG(LIB_LVL, CERROR, "stack_realloc failed");
        return 1;
    }
    copy(new_item, (void*) &st->contents[char_index(st->top, st->obj_size)]);
    st->top++;
    return 0;
}

int emplace_bpush(struct bstack* st, void (*init) (void* item))
{
    if (stack_realloc(st) != 0)
    {
        LOG(LIB_LVL, CERROR, "stack_realloc failed");
        return 1;
    }
    init((void*) &st->contents[char_index(st->top, st->obj_size)]);
    st->top++;
    return 0;
}

int bpop(struct bstack* st, void* result)
{
    if (bstack_empty(st))
        return 1;
    st->top--; 
    memcpy(result, &st->contents[char_index(st->top, st->obj_size)], st->obj_size);
    return 0;
}

int btop(const struct bstack* st, void* result)
{
    if (bstack_empty(st))
        return 1;
    memcpy(result, &st->contents[char_index(st->top - 1, st->obj_size)], st->obj_size);
    return 0;
}

int bstack_empty(const struct bstack* st)
{
    return st->top == 0;
}

size_t bstack_size(const struct bstack* st)
{
    return st->top;
}

void bstack_walk(struct bstack* st, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = bstack_size(st); i-- > 0; )
        handler((void*)&st->contents[char_index(i, st->obj_size)], userdata);
}

void bstack_free(struct bstack* st, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        bstack_walk(st, userdata, deallocator);
    free(st->contents);
    st->contents = NULL;
    st->capacity = 2; st->top= 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}

static int stack_realloc(struct bstack* st)
{
    if (st->capacity == bstack_size(st))
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
    return 0;
}