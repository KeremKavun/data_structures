#include <ds/stack/lstack.h>
#include <ds/linkedlists/slist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stack_item {
    void                    *data;
    struct slist_item       hook;
};

struct lstack {
    struct slist                    contents;
    struct allocator_concept        ac;
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
    slist_init(&ls->contents);
    ls->ac = *ac;
    return ls;
}

void lstack_destroy(struct lstack *ls, deinit_cb deinit)
{
    assert(ls != NULL);
    void *data;
    while ((data = lpop(ls))) {
        if (deinit)
            deinit(data);
    }
    free(ls);
}

size_t lstack_node_sizeof()
{
    return sizeof(struct stack_item);
}

/* =========================================================================
 * Push & Pop
 * ========================================================================= */

int lpush(struct lstack *ls, void *new_item)
{
    assert(ls != NULL);
    struct stack_item *sti = ls->ac.alloc(ls->ac.allocator);
    if (!sti)
        return 1;
    sti->data = new_item;
    slist_insert(&ls->contents, slist_head(&ls->contents), &sti->hook);
    return 0;
}

void *lpop(struct lstack *ls)
{
    assert(ls != NULL);
    struct slist_item** head = slist_head(&ls->contents);
    if (*head == NULL)
        return NULL;
    struct slist_item *to_remove = *head;
    slist_remove(&ls->contents, head);
    struct stack_item *sti = slist_entry(to_remove, struct stack_item, hook);
    void *data = sti->data;
    ls->ac.free(ls->ac.allocator, sti);
    return data;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

void *ltop(struct lstack *ls)
{
    assert(ls != NULL);
    struct slist_item* peek = *slist_head(&ls->contents);
    return (peek) ? slist_entry(peek, struct stack_item, hook)->data : NULL;
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
    slist_foreach(iter, slist_head(&ls->contents), NULL) {
        struct stack_item *parent = slist_entry(*iter, struct stack_item, hook);
        handler(parent->data, context);
    }
}