#include "../include/dlist.h"
#include <stdlib.h>

// *** dlist_item implementation *** //

void dlist_item_init(struct dlist_item *item, struct dlist_item *prev, struct dlist_item *next, void *data)
{
    assert(item != NULL);
    item->prev = prev;
    item->next = next;
    item->data = data;
}

// *** dlist implementation *** //

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

void dlist_init(struct dlist *dl, struct allocator_concept *ac)
{
    assert(dl != NULL);
    assert(ac != NULL);
    dlist_item_init(&dl->sentinel, &dl->sentinel, &dl->sentinel, NULL);
    dl->size = 0;
    dl->ac = *ac;
}

void dlist_deinit(struct dlist *dl, struct object_concept *oc)
{
    assert(dl != NULL);
    struct dlist_item *curr;
    struct dlist_item *n;
    dlist_foreach_fr_safe(curr, n, dlist_head(dl), &dl->sentinel) {
        void *data = dlist_remove(dl, curr);
        if (oc && oc->deinit)
            oc->deinit(data);
    }
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

int dlist_insert_between(struct dlist *dl, struct dlist_item *prev_node, struct dlist_item *next_node, void *new_data)
{
    assert(dl != NULL);
    assert(prev_node != NULL && next_node != NULL);
    assert(dl->ac.alloc != NULL && dl->ac.allocator != NULL);
    struct dlist_item *new_item = dl->ac.alloc(dl->ac.allocator);
    if (!new_item) {
        LOG(LIB_LVL, CERROR, "Allocator failed");
        return 1;
    }
    dlist_item_init(new_item, prev_node, next_node, new_data);
    // Sentinel usage assures prev_node->next && next_node->prev are non-NULL.
    prev_node->next = new_item;
    next_node->prev = new_item;
    dl->size++;
    return 0;
}

int dlist_push_front(struct dlist *dl, void *new_data)
{
    return dlist_insert_between(dl, &dl->sentinel, dl->sentinel.next, new_data);
}

int dlist_push_back(struct dlist *dl, void *new_data)
{
    return dlist_insert_between(dl, dl->sentinel.prev, &dl->sentinel, new_data);
}

/* =========================================================================
 * Removal
 * ========================================================================= */

void *dlist_remove(struct dlist *dl, struct dlist_item *item)
{
    assert(dl != NULL);
    assert(item != NULL);
    assert(dl->ac.free != NULL && dl->ac.allocator != NULL);
    if (item == &dl->sentinel)
        return NULL; 
    struct dlist_item *prev = item->prev;
    struct dlist_item *next = item->next;
    prev->next = next;
    next->prev = prev;
    void* data = item->data;
    dl->ac.free(dl->ac.allocator, item);
    dl->size--;
    return data;
}

void *dlist_remove_front(struct dlist *dl)
{
    assert(dl != NULL);
    return dlist_remove(dl, dlist_head(dl));
}

void *dlist_remove_back(struct dlist *dl)
{
    assert(dl != NULL);
    return dlist_remove(dl, dlist_tail(dl));
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

int dlist_empty(const struct dlist *dl)
{
    assert(dl != NULL);
    return (dl->size == 0);
}

size_t dlist_size(const struct dlist *dl)
{
    assert(dl != NULL);
    return dl->size;
}

struct dlist_item *dlist_head(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.next;
}

struct dlist_item *dlist_tail(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.prev;
}

// Others

void dlist_reverse(struct dlist *dl)
{
    struct dlist_item *item;
    struct dlist_item *n;
    dlist_foreach_fr_safe(item, n, dlist_head(dl), &dl->sentinel) {
        struct dlist_item *tmp =  item->next;
        item->next = item->prev;
        item->prev = tmp;
    }
    // Dont forget to handle the sentinel!
    struct dlist_item *tmp = dl->sentinel.next;
    dl->sentinel.next = dl->sentinel.prev;
    dl->sentinel.prev = tmp;
}