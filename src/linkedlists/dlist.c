#include <ds/linkedlists/dlist.h>
#include <stdlib.h>

static void dlist_insert_between(struct dlist *dl, struct dlist_item *prev_node, struct dlist_item *next_node, struct dlist_item *new_item);

// *** dlist_item implementation *** //

void dlist_item_init(struct dlist_item *item, struct dlist_item *prev, struct dlist_item *next)
{
    assert(item != NULL);
    item->prev = prev;
    item->next = next;
}

// *** dlist implementation *** //

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

void dlist_init(struct dlist *dl)
{
    assert(dl != NULL);
    dlist_item_init(&dl->sentinel, &dl->sentinel, &dl->sentinel);
    dl->size = 0;
}

void dlist_deinit(struct dlist *dl, deinit_cb deinit)
{
    assert(dl != NULL);
    if (!deinit) {
        dlist_item_init(&dl->sentinel, &dl->sentinel, &dl->sentinel);
        dl->size = 0;
        return;
    }
    struct dlist_item *item;
    while ((item = dlist_remove_front(dl)) != NULL) {
        deinit(item);
    }
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

void dlist_insert_after(struct dlist *dl, struct dlist_item *pos, struct dlist_item *new_item)
{
    assert(pos != NULL);
    dlist_insert_between(dl, pos, pos->next, new_item);
}

void dlist_insert_before(struct dlist *dl, struct dlist_item *pos, struct dlist_item *new_item)
{
    assert(pos != NULL);
    dlist_insert_between(dl, pos->prev, pos, new_item);
}

void dlist_push_front(struct dlist *dl, struct dlist_item *new_item)
{
    dlist_insert_between(dl, &dl->sentinel, dl->sentinel.next, new_item);
}

void dlist_push_back(struct dlist *dl, struct dlist_item *new_item)
{
    dlist_insert_between(dl, dl->sentinel.prev, &dl->sentinel, new_item);
}

/* =========================================================================
 * Removal
 * ========================================================================= */

struct dlist_item *dlist_remove(struct dlist *dl, struct dlist_item *item)
{
    assert(dl != NULL);
    assert(item != NULL);
    if (item == &dl->sentinel)
        return NULL; 
    struct dlist_item *prev = item->prev;
    struct dlist_item *next = item->next;
    prev->next = next;
    next->prev = prev;
    dlist_item_init(item, NULL, NULL);
    dl->size--;
    return item;
}

struct dlist_item *dlist_remove_front(struct dlist *dl)
{
    return dlist_remove(dl, dlist_head(dl));
}

struct dlist_item *dlist_remove_back(struct dlist *dl)
{
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

// *** Helper functions *** //

static void dlist_insert_between(struct dlist *dl, struct dlist_item *prev_node, struct dlist_item *next_node, struct dlist_item *new_item)
{
    assert(dl != NULL);
    assert(prev_node != NULL && next_node != NULL);
    dlist_item_init(new_item, prev_node, next_node);
    // Sentinel usage assures prev_node->next && next_node->prev are non-NULL.
    prev_node->next = new_item;
    next_node->prev = new_item;
    dl->size++;
}