#include "../include/lstack.h"
#include "../../linkedlists/include/slist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t node_size = sizeof(struct slist_item);

struct lstack {
    struct slist        contents;
};

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

struct lstack *lstack_create(struct allocator_concept *ac)
{
    assert(ac != NULL);
    struct lstack *ls = malloc(sizeof(struct lstack));
    if (!ls) {
        LOG(LIB_LVL, CERROR, "Could not allocate lstack");
        return NULL;
    }
    slist_init(&ls->contents, ac);
    return ls;
}

void lstack_destroy(struct lstack *ls, struct object_concept *oc)
{
    assert(ls != NULL);
    void *data;
    while ((data = lpop(ls))) {
        if (oc && oc->deinit)
            oc->deinit(data);
    }
    free(ls);
}

/* =========================================================================
 * Push & Pop
 * ========================================================================= */

int lpush(struct lstack *ls, void *new_item)
{
    assert(ls != NULL);
    return slist_insert(&ls->contents, slist_head(&ls->contents), new_item);
}

void *lpop(struct lstack *ls)
{
    assert(ls != NULL);
    struct slist_item** head = slist_head(&ls->contents);
    return (*head) ? slist_remove(&ls->contents, head) : NULL;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

void *ltop(struct lstack *ls)
{
    assert(ls != NULL);
    struct slist_item** peek = slist_head(&ls->contents);
    return (*peek) ? (*peek)->data : NULL;
}

int lstack_empty(const struct lstack *ls)
{
    assert(ls != NULL);
    return slist_empty(&ls->contents);
}

size_t lstack_size(const struct lstack *ls)
{
    assert(ls != NULL);
    return slist_size(&ls->contents);
}

/* =========================================================================
 * Iterations
 * ========================================================================= */

void lstack_walk(struct lstack *ls, void *context, void (*handler) (void *item, void *context))
{
    assert(ls);
    struct slist_item **iter;
    slist_foreach(&ls->contents, iter, slist_head(&ls->contents), NULL) {
        void *data = slist_item_data(iter);
        handler(data, context);
    }
}