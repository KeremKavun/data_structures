#include <ds/linkedlists/clist.h>
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

void clist_init(struct clist *cl) {
    assert(cl != NULL);
    clist_item_init(&cl->sentinel);
    cl->size = 0;
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

void clist_insert_before(struct clist* cl, struct clist_item* pos, struct clist_item* new_item)
{
    /*
     * Logic: Insert [N] before [P]
     * Before: [Prev] <-> [P]
     * After:  [Prev] <-> [N] <-> [P]
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
     * Before: [P] <-> [Next]
     * After:  [P] <-> [N] <-> [Next]
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

void clist_push_front(struct clist *cl, struct clist_item *new_item) {
    clist_insert_after(cl, &cl->sentinel, new_item);
}

void clist_push_back(struct clist *cl, struct clist_item *new_item) {
    clist_insert_before(cl, &cl->sentinel, new_item);
}

/* =========================================================================
 * Removal
 * ========================================================================= */

void clist_remove(struct clist* cl, struct clist_item* item)
{
    assert(cl != NULL);
    assert(item != NULL);
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next = NULL; 
    item->prev = NULL;
    cl->size--;
}

struct clist_item* clist_pop_front(struct clist* cl)
{
    assert(cl != NULL);
    if (clist_empty(cl))
        return NULL;
    struct clist_item* del = cl->sentinel.next;
    clist_remove(cl, del);
    return del;
}

struct clist_item* clist_pop_back(struct clist* cl)
{
    assert(cl != NULL);
    if (clist_empty(cl))
        return NULL;
    struct clist_item* del = cl->sentinel.prev;
    clist_remove(cl, del);
    return del;
}