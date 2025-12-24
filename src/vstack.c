#include "../include/vstack.h"
#include "../../arrays/include/dynarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITAL_CAPACITY 2

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

int vstack_init(struct vstack *vs, size_t obj_size, struct object_concept *oc)
{
    assert(vs != NULL);
    assert(obj_size != 0);
    assert(oc != NULL);
    struct dynarray* contents = malloc(sizeof(struct dynarray));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    vs->contents = contents;
    dynarray_init(vs->contents, INITAL_CAPACITY, obj_size, *oc);
    return 0;
}

void vstack_deinit(struct vstack *vs)
{
    assert(vs != NULL);
    dynarray_deinit(vs->contents);
    free(vs->contents);
    vs->contents = NULL;
}

/* =========================================================================
 * Push & Pop
 * ========================================================================= */

int vpush(struct vstack *vs, void *new_item)
{
    assert(vs != NULL);
    return dynarray_push_back(vs->contents, new_item);
}

int vpop(struct vstack *vs, void *popped_item)
{
    assert(vs != NULL);
    if (dynarray_empty(vs->contents))
        return -1;
    if (popped_item) {
        int result = vs->contents->oc.init(popped_item, dynarray_back(vs->contents));
        if (result != 0)
            return result;
    }
    dynarray_pop_back(vs->contents);
    return 0;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

int vtop(struct vstack *vs, void *top_item)
{
    assert(vs != NULL);
    if (dynarray_empty(vs->contents))
        return -1;
    return vs->contents->oc.init(top_item, dynarray_back(vs->contents));
}

int vstack_empty(const struct vstack *vs)
{
    assert(vs != NULL);
    return dynarray_empty(vs->contents);
}

size_t vstack_size(const struct vstack *vs)
{
    assert(vs != NULL);
    return dynarray_size(vs->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void vstack_walk(struct vstack *vs, void *context, void (*handler) (void *item, void *context))
{
    assert(vs && vs->contents);
    void *begin = dynarray_iterator_begin(vs->contents);
    void *end = dynarray_iterator_end(vs->contents);
    while (begin != end) {
        handler(begin, context);
        begin = dynarray_iterator_next(vs->contents, begin);
    }
}