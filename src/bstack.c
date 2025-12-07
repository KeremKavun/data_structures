#include "../include/bstack.h"
#include "../../buffers/include/lbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int bstack_init(struct bstack* bs, size_t obj_size)
{
    struct lbuffer* contents = malloc(sizeof(struct lbuffer));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    if (lbuffer_init(contents, INITIAL_CAPACITY, obj_size) != 0)
    {
        LOG(LIB_LVL, CERROR, "Buffer couldnt be initialized");
        return 1;
    }
    bs->contents = contents;
    return 0;
}

void bstack_deinit(struct bstack* bs, void* context, struct object_concept* oc)
{
    lbuffer_deinit(bs->contents, context, oc);
    free(bs->contents);
    bs->contents = NULL;
}

/*───────────────────────────────────────────────
 * Push & Pop
 *───────────────────────────────────────────────*/

int bpush(struct bstack* bs, const void* new_item)
{
    LOG(LIB_LVL, CINFO, "Pushing item at address %p, by copying", new_item);
    return lbuffer_insert(bs->contents, new_item, bstack_size(bs));
}

int emplace_bpush(struct bstack* bs, void* args, struct object_concept* oc)
{
    LOG(LIB_LVL, CINFO, "Pushing item by emplacing");
    return lbuffer_emplace_insert(bs->contents, bstack_size(bs), args, oc);
}

int bpop(struct bstack* bs, void* result)
{
    LOG(LIB_LVL, CINFO, "Popping item");
    return lbuffer_remove(bs->contents, bstack_size(bs) - 1, result);
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int btop(const struct bstack* bs, void* result)
{
    return lbuffer_get(bs->contents, bstack_size(bs) - 1, result);
}

int bstack_empty(const struct bstack* bs)
{
    return lbuffer_empty(bs->contents);
}

size_t bstack_size(const struct bstack* bs)
{
    return lbuffer_size(bs->contents);
}

size_t bstack_capacity(const struct bstack* bs)
{
    return lbuffer_capacity(bs->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bstack_walk(struct bstack* bs, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = bstack_size(bs); i-- > 0; )
        handler(lbuffer_at(bs->contents, i), userdata);
}