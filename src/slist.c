#include "../include/slist.h"
#include <stdlib.h>

// *** list_item implementation *** //

void slist_item_init(struct slist_item *item, struct slist_item *next, void *data)
{
    assert(item != NULL);
    item->next = next;
    item->data = data;
}

// *** linked_list implementation *** //

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

void slist_init(struct slist *sl, struct allocator_concept *ac)
{
    assert(sl != NULL);
    assert(ac != NULL);
    slist_item_init(&sl->sentinel, NULL, NULL);
    sl->size = 0;
    sl->ac = ac;
}

void slist_deinit(struct slist *sl, void *context, struct object_concept *oc)
{
    assert(sl != NULL);
    while (!slist_empty(sl)) {
        void *data = slist_remove(sl, slist_head(sl));
        if (oc && oc->deinit)
            oc->deinit(data, context);
    }
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

int slist_insert(struct slist *sl, struct slist_item **pos, void *new_data)
{
    struct slist_item *new_item = sl->ac->alloc(sl->ac->allocator);
    if (!new_item) {
        LOG(LIB_LVL, CERROR, "Allocator failed");
        return 1;
    }
    struct slist_item *curr_item = *pos;
    *pos = new_item;
    slist_item_init(new_item, curr_item, new_data);
    sl->size++;
    return 0;
}

/* =========================================================================
 * Removal
 * ========================================================================= */

void *slist_remove(struct slist *sl, struct slist_item **item)
{
    assert(sl != NULL);
    struct slist_item *del = *item;
    if (del == &sl->sentinel)
        return NULL; 
    *item = del->next;
    void *data = del->data;
    sl->ac->free(sl->ac->allocator, del);
    sl->size--;
    return data;
}

/* =========================================================================
 * Iteration
 * ========================================================================= */

/* =========================================================================
 * Search
 * ========================================================================= */

/* =========================================================================
 * Inspection
 * ========================================================================= */

int slist_empty(const struct slist *sl)
{
    assert(sl != NULL);
    return (sl->size == 0);
}

size_t slist_size(const struct slist *sl)
{
    assert(sl != NULL);
    return sl->size;
}

struct slist_item **slist_head(struct slist *sl)
{
    assert(sl != NULL);
    return &sl->sentinel.next;
}