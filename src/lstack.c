#include "../include/lstack.h"
#include "../../linkedlists/include/slist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t node_size = sizeof(struct slist_item);

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

int lstack_init(struct lstack* ls, struct allocator_concept* ac)
{
    struct slist* contents = malloc(sizeof(struct slist));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    ls->contents = contents;
    slist_init(ls->contents, ac);
    return 0;
}

void lstack_deinit(struct lstack* ls, struct object_concept* oc)
{
    void* data;
    while ((data = lpop(ls)))
    {
        if (oc && oc->deinit)
            oc->deinit(data);
    }
    free(ls->contents);
    ls->contents = NULL;
}

/* =========================================================================
 * Push & Pop
 * ========================================================================= */

int lpush(struct lstack* ls, void* new_item)
{
    return slist_insert(ls->contents, slist_head(ls->contents), new_item);
}

void* lpop(struct lstack* ls)
{
    struct slist_item** head = slist_head(ls->contents);
    return (*head) ? slist_remove(ls->contents, head) : NULL;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

void* ltop(struct lstack* ls)
{
    struct slist_item** peek = slist_head(ls->contents);
    return (*peek) ? (*peek)->data : NULL;
}

int lstack_empty(const struct lstack* ls)
{
    return slist_empty(ls->contents);
}

size_t lstack_size(const struct lstack* ls)
{
    return slist_size(ls->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void lstack_walk(struct lstack* ls, void* context, void (*handler) (void* item, void* context))
{
    assert(ls && ls->contents);
    struct slist_item **iter;
    slist_foreach(ls->contents, iter, slist_head(ls->contents), NULL) {
        void *data = slist_item_data(iter);
        handler(data, context);
    }
}