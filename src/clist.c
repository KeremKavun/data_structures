#include "../include/clist.h"
#include <stdlib.h>
#include <assert.h>

// *** clist_item implementation *** //

void clist_item_init(struct clist_item* item)
{
    assert(item != NULL);
    item->prev = item;
    item->next = item;
}

// *** clist implementation *** //

/* =========================================================================
 * Initialization
 * ========================================================================= */

void clist_init(struct clist* cl)
{
    assert(cl != NULL);
    cl->cursor = NULL;
    cl->size = 0;
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

void clist_insert_before(struct clist* cl, struct clist_item* pos, struct clist_item* new_item)
{
    /*
     * Logic: Insert [N] before [S]
     * Before: [P] <-> [S]
     * After:  [P] <-> [N] <-> [S]
     */

    assert(cl != NULL);
    assert(pos != NULL);
    assert(new_item != NULL);
    new_item->next = pos;
    new_item->prev = pos->prev;
    pos->prev->next = new_item;
    pos->prev = new_item;
    cl->size++;
}

void clist_insert_after(struct clist* cl, struct clist_item* pos, struct clist_item* new_item)
{
    /*
     * Logic: Insert [N] after [P]
     * Before: [P] <-> [S]
     * After:  [P] <-> [N] <-> [S]
     */

    assert(cl != NULL);
    assert(pos != NULL);
    assert(new_item != NULL);
    new_item->prev = pos;
    new_item->next = pos->next;
    pos->next->prev = new_item;
    pos->next = new_item;
    cl->size++;
}

void clist_push_front(struct clist* cl, struct clist_item* new_item)
{
    assert(cl != NULL);
    assert(new_item != NULL);
    if (clist_empty(cl))
    {
        cl->cursor = new_item;
        cl->size++;
    }
    else
    {
        clist_insert_before(cl, cl->cursor, new_item);
        cl->cursor = new_item;
    }
}

void clist_push_back(struct clist* cl, struct clist_item* new_item)
{
    assert(cl != NULL);
    assert(new_item != NULL);
    if (clist_empty(cl))
    {
        cl->cursor = new_item;
        cl->size++;
    }
    else
        clist_insert_before(cl, cl->cursor, new_item);
}

/* =========================================================================
 * Removal
 * ========================================================================= */

void clist_remove(struct clist* cl, struct clist_item* item)
{
    assert(cl != NULL);
    assert(item != NULL);
    // Only one item in the circle. It points to itself, so we just clear the list.
    if (cl->size == 1)
        cl->cursor = NULL;
    else
    {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        // If we are removing the entry point, move it to the next valid node
        if (cl->cursor == item)
            cl->cursor = item->next;
    }
    cl->size--;
}

struct clist_item* clist_pop_front(struct clist* cl)
{
    assert(cl != NULL);
    if (clist_empty(cl))
        return NULL;
    struct clist_item* del = get_clist_cursor(cl);
    clist_remove(cl, del);
    return del;
}

struct clist_item* clist_pop_back(struct clist* cl)
{
    assert(cl != NULL);
    if (clist_empty(cl))
        return NULL;
    struct clist_item* cursor = get_clist_cursor(cl);
    struct clist_item* del = cursor->prev;
    clist_remove(cl, del);
    return del;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

struct clist_item* get_clist_cursor(struct clist* cl)
{
    assert(cl != NULL);
    return cl->cursor;
}

int clist_empty(const struct clist* cl)
{
    assert(cl != NULL);
    return (cl->size == 0);
}

size_t clist_size(const struct clist* cl)
{
    assert(cl != NULL);
    return cl->size;
}

// Others

void set_clist_cursor(struct clist* cl, struct clist_item* new_cursor)
{
    assert(cl != NULL);
    cl->cursor = new_cursor;
}