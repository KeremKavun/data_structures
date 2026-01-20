#include <ds/linkedlists/slist.h>
#include <stdlib.h>

// *** list_item implementation *** //

// *** linked_list implementation *** //

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

void slist_init(struct slist *sl)
{
    assert(sl != NULL);
    slist_item_init(&sl->sentinel, NULL);
    sl->size = 0;
}

void slist_deinit(struct slist *sl, deinit_cb deinit)
{
    assert(sl != NULL);
    if (!deinit) {
        sl->sentinel.next = NULL;
        slist_item_init(&sl->sentinel, NULL);
        sl->size = 0;
        return;
    }
    while (!slist_empty(sl)) {
        struct slist_item **head_ptr = slist_head(sl);
        struct slist_item *node_to_free = *head_ptr;
        slist_remove(sl, head_ptr);
        deinit(node_to_free);
    }
}

/* =========================================================================
 * Insertion & Removal
 * ========================================================================= */

void slist_insert(struct slist *sl, struct slist_item **pos, struct slist_item *new_item)
{
    assert(sl != NULL && pos != NULL && new_item != NULL);
    struct slist_item *curr_item = *pos;
    *pos = new_item;
    slist_item_init(new_item, curr_item);
    sl->size++;
}

void slist_remove(struct slist *sl, struct slist_item **item)
{
    assert(sl != NULL && item != NULL);
    struct slist_item *del = *item;
    if (del == &sl->sentinel)
        return; 
    *item = del->next;
    slist_item_init(del, NULL);
    sl->size--;
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