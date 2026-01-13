#include <ds/stack/vstack.h>
#include <ds/arrays/dynarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITAL_CAPACITY 2

struct vstack {
    struct dynarray         contents;
};

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

struct vstack *vstack_create(size_t obj_size, struct object_concept *oc)
{
    assert(obj_size != 0);
    assert(oc != NULL);
    struct vstack *vs = malloc(sizeof(struct vstack));
    if (!vs) {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return NULL;
    }
    if (dynarray_init(&vs->contents, INITAL_CAPACITY, obj_size, *oc) != 0) {
        LOG(LIB_LVL, CERROR, "Could not initialize underlying dynarray");
        free(vs);
        return NULL;
    }
    return vs;
}

void vstack_destroy(struct vstack *vs)
{
    assert(vs != NULL);
    dynarray_deinit(&vs->contents);
    free(vs);
}

/* =========================================================================
 * Push & Pop
 * ========================================================================= */

int vpush(struct vstack *vs, void *new_item)
{
    assert(vs != NULL);
    return dynarray_push_back(&vs->contents, new_item);
}

int vpop(struct vstack *vs, void *popped_item)
{
    assert(vs != NULL);
    if (dynarray_empty(&vs->contents))
        return -1;
    if (popped_item) {
        int result = vs->contents.oc.init(popped_item, dynarray_back(&vs->contents));
        if (result != 0)
            return result;
    }
    dynarray_pop_back(&vs->contents);
    return 0;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

int vtop(struct vstack *vs, void *top_item)
{
    assert(vs != NULL);
    void *data = dynarray_back(&vs->contents);
    if (data == NULL)
        return -1;
    return vs->contents.oc.init(top_item, dynarray_back(&vs->contents));
}

int vstack_empty(const struct vstack *vs)
{
    assert(vs != NULL);
    return dynarray_empty(&vs->contents);
}

size_t vstack_size(const struct vstack *vs)
{
    assert(vs != NULL);
    return dynarray_size(&vs->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void vstack_walk(struct vstack *vs, void *context, void (*handler) (void *item, void *context))
{
    assert(vs);
    void *begin = dynarray_iterator_begin(&vs->contents);
    void *end = dynarray_iterator_end(&vs->contents);
    while (begin != end) {
        handler(begin, context);
        begin = dynarray_iterator_next(&vs->contents, begin);
    }
}