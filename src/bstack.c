#include "../include/bstack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

static size_t char_index(size_t ind, size_t obj_size);
static int stack_realloc(struct bstack* bs);

int bstack_init(struct bstack* bs, size_t obj_size)
{
    bs->contents = malloc(sizeof(char) * obj_size * INITIAL_CAPACITY);
    if (!bs->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    bs->capacity = INITIAL_CAPACITY;
    bs->size= 0;
    bs->obj_size = obj_size;
    return 0;
}

int bpush(struct bstack* bs, const void* new_item, void* userdata, int (*copy) (const void* new_item, void* queue_item, void* userdata))
{
    if (stack_realloc(bs) != 0)
    {
        LOG(LIB_LVL, CERROR, "stack_realloc failed");
        return 1;
    }
    if (copy(new_item, (void*) &bs->contents[char_index(bstack_size(bs), bs->obj_size)], userdata) != 0)
    {
        LOG(PROJ_LVL, CERROR, "Copy ctor failed");
        return 1;
    }
    bs->size++;
    return 0;
}

int emplace_bpush(struct bstack* bs, void* userdata, int (*init) (void* item, void* userdata))
{
    if (stack_realloc(bs) != 0)
    {
        LOG(LIB_LVL, CERROR, "stack_realloc failed");
        return 1;
    }
    if (init((void*) &bs->contents[char_index(bstack_size(bs), bs->obj_size)], userdata) != 0)
    {
        LOG(PROJ_LVL, CERROR, "Initializer failed");
        return 1;
    }
    bs->size++;
    return 0;
}

int bpop(struct bstack* bs, void* result)
{
    if (bstack_empty(bs))
        return 1;
    bs->size--;
    if (result)
        memcpy(result, &bs->contents[char_index(bstack_size(bs), bs->obj_size)], bs->obj_size);
    return 0;
}

int btop(const struct bstack* bs, void* result)
{
    if (bstack_empty(bs))
        return 1;
    memcpy(result, &bs->contents[char_index(bstack_size(bs) - 1, bs->obj_size)], bs->obj_size);
    return 0;
}

int bstack_empty(const struct bstack* bs)
{
    return bs->size == 0;
}

size_t bstack_size(const struct bstack* bs)
{
    return bs->size;
}

size_t bstack_capacity(const struct bstack* bs)
{
    return bs->capacity;
}

void bstack_walk(struct bstack* bs, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = bstack_size(bs); i-- > 0; )
        handler((void*)&bs->contents[char_index(i, bs->obj_size)], userdata);
}

void bstack_free(struct bstack* bs, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        bstack_walk(bs, userdata, deallocator);
    free(bs->contents);
    bs->contents = NULL;
    bs->capacity = 2; bs->size= 0;
}

// *** Helper functions *** //

static inline size_t char_index(size_t ind, size_t obj_size)
{
    return ind * obj_size;
}

static int stack_realloc(struct bstack* bs)
{
    if (bstack_capacity(bs)== bstack_size(bs))
    {
        char* new_contents = realloc(bs->contents, sizeof(char) * bs->obj_size * bstack_capacity(bs) * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        bs->contents = new_contents;
        bs->capacity *= 2;
    }
    return 0;
}