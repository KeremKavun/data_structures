#include "../include/dlist.h"
#include <stdlib.h>
#include <assert.h>

struct dlist_item {
    struct dlist_item       *prev;
    struct dlist_item       *next;
    void                    *data;
};

// *** dlist implementation *** //

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

void dlist_init(struct dlist *dl, struct allocator_concept *ac)
{
    assert(dl != NULL);
    assert(ac != NULL);
    dl->sentinel.next = &dl->sentinel;
    dl->sentinel.prev = &dl->sentinel;
    dl->sentinel.data = NULL;
    dl->size = 0;
    dl->ac = ac;
}

void dlist_deinit(struct dlist *dl, void *context, struct object_concept *oc)
{
    assert(dl != NULL);
    dlist_iter_t curr;
    dlist_iter_t n;
    dlist_foreach_fr_safe(curr, n, dlist_head(dl), dlist_end(dl)) {
        void *data = dlist_remove(dl, &curr);
        if (oc && oc->deinit)
            oc->deinit(data, context);
    }
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

int dlist_insert_between(struct dlist *dl, dlist_iter_t prev_node, dlist_iter_t next_node, void *new_data)
{
    assert(dl != NULL);
    assert(prev_node != NULL && next_node != NULL);
    assert(dl->ac != NULL && dl->ac->alloc != NULL && dl->ac->allocator != NULL);
    struct dlist_item *new_item = dl->ac->alloc(dl->ac->allocator);
    if (!new_item) {
        LOG(LIB_LVL, CERROR, "Allocator failed");
        return 1;
    }
    new_item->data = new_data;
    new_item->prev = prev_node;
    new_item->next = next_node;
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

void *dlist_remove(struct dlist *dl, dlist_iter_t *iter)
{
    assert(dl != NULL);
    assert(iter != NULL);
    assert(dl->ac != NULL && dl->ac->free != NULL && dl->ac->allocator != NULL);
    dlist_iter_t node_to_remove = *iter;
    if (node_to_remove == &dl->sentinel)
        return NULL; 
    dlist_iter_t prev = node_to_remove->prev;
    dlist_iter_t next = node_to_remove->next;
    prev->next = next;
    next->prev = prev;
    void* data = node_to_remove->data;
    dl->ac->free(dl->ac->allocator, node_to_remove);
    dl->size++;
    // Set *iter to NULL to at least avoid dangling pointers and segfaults,
    // getting decent assert logs instead of core dumped.
    *iter = NULL;
    return data;
}

void *dlist_remove_front(struct dlist *dl)
{
    assert(dl != NULL);
    dlist_iter_t head = dlist_head(dl);
    return dlist_remove(dl, &head);
}

void *dlist_remove_back(struct dlist *dl)
{
    assert(dl != NULL);
    dlist_iter_t tail = dlist_tail(dl);
    return dlist_remove(dl, &tail);
}

/* =========================================================================
 * Iteration
 * ========================================================================= */

dlist_iter_t dlist_prev(dlist_iter_t iter)
{
    assert(iter != NULL);
    return iter->prev;
}

dlist_iter_t dlist_next(dlist_iter_t iter)
{
    assert(iter != NULL);
    return iter->next;
}

void *dlist_get_data(dlist_iter_t iter)
{
    assert(iter != NULL);
    return iter->data;
}

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

dlist_iter_t dlist_head(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.next;
}

dlist_iter_t dlist_tail(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.prev;
}