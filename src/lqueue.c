#include "../include/lqueue.h"
#include "../../linked_lists/include/dbly_linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int lqueue_init(struct lqueue* lq, struct allocator_concept* ac)
{
    struct dbly_linked_list* contents = malloc(sizeof(struct dbly_linked_list));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    lq->contents = contents;
    dbly_list_init(lq->contents);
    lq->ac = ac;
    return 0;
}

void lqueue_deinit(struct lqueue* lq, void* context, struct object_concept* oc)
{
    dbly_list_free(lq->contents, context, oc);
    free(lq->contents);
    lq->contents = NULL;
    lq->ac = NULL;
}

/*───────────────────────────────────────────────
 * Enqueue & Dequeue
 *───────────────────────────────────────────────*/

int lenqueue(struct lqueue* lq, void* new_item)
{
    struct dbly_list_item* new_node = (lq->ac && lq->ac->allocator) ? lq->ac->alloc(lq->ac->allocator) : malloc(sizeof(struct dbly_list_item));
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    dbly_list_item_init(new_node, new_item);
    dbly_list_insert_back(lq->contents, new_node);
    return 0;
}

void* ldequeue(struct lqueue* lq)
{
    struct dbly_list_item* del = dbly_list_remove_front(lq->contents);
    if (!del)
        return NULL;
    void* data = del->data;
    (lq->ac && lq->ac->allocator) ? lq->ac->free(lq->ac->allocator, del) : free(del);
    return data;
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

void* lqueue_front(const struct lqueue* lq)
{
    struct dbly_list_item* head = dbly_list_head(lq->contents);
    return head ? head->data : NULL;
}

void* lqueue_rear(const struct lqueue* lq)
{
    struct dbly_list_item* tail = dbly_list_tail(lq->contents);
    return tail ? tail->data : NULL;
}

int lqueue_empty(const struct lqueue* lq)
{
    return dbly_list_empty(lq->contents);
}

size_t lqueue_size(const struct lqueue* lq)
{
    return dbly_list_size(lq->contents);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void lqueue_walk(const struct lqueue* lq, void* context, void (*handler) (void* item, void* context))
{
    dbly_list_walk_front(lq->contents, context, handler);
}