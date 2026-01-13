#include <ds/queue/lqueue.h>
#include <ds/linkedlists/slist.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const size_t queue_node_size = sizeof(struct slist_item);

struct lqueue {
    struct slist            contents;
    struct slist_item       **rear;
};

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

struct lqueue *lqueue_create(struct allocator_concept *ac)
{
    assert(ac != NULL);
    struct lqueue *lq = malloc(sizeof(struct lqueue));
    if (!lq) {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return NULL;
    }
    slist_init(&lq->contents, ac);
    lq->rear = slist_head(&lq->contents);
    return lq;
}

void lqueue_destroy(struct lqueue *lq, struct object_concept *oc)
{
    assert(lq != NULL);
    slist_deinit(&lq->contents, oc);
    free(lq);
}

/* =========================================================================
 * Enqueue & Dequeue
 * ========================================================================= */

int lenqueue(struct lqueue *lq, void *new_item)
{
    assert(lq != NULL);
    if (slist_insert(&lq->contents, lq->rear, new_item) != 0)
        return 1;
    lq->rear = slist_item_next(lq->rear);
    return 0;
}

void *ldequeue(struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item **head = slist_head(&lq->contents);
    if (*head == NULL)
        return NULL;
    void *data = slist_remove(&lq->contents, head);
    // 4. FIX: If the list is now empty, 'rear' is dangling!
    // We must reset 'rear' to point to the sentinel's next field.
    if (lqueue_empty(lq))
        lq->rear = slist_head(&lq->contents);
    return data;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

void *lqueue_front(const struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item **front = slist_head((struct slist *) &lq->contents);
    return (*front) ? (*front)->data : NULL;
}

void *lqueue_rear(const struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item *real_rear = (struct slist_item *) lq->rear;
    return (real_rear) ? real_rear->data : NULL;
}

int lqueue_empty(const struct lqueue *lq)
{
    assert(lq != NULL);
    return slist_empty(&lq->contents);
}

size_t lqueue_size(const struct lqueue *lq)
{
    assert(lq != NULL);
    return slist_size(&lq->contents);
}

/* =========================================================================
 * Iterations
 * ========================================================================= */

void lqueue_walk(struct lqueue *lq, void *context, void (*handler) (void *item, void *context))
{
    assert(lq);
    struct slist_item **iter;
    slist_foreach(&lq->contents, iter, slist_head(&lq->contents), NULL) {
        void *data = slist_item_data(iter);
        handler(data, context);
    }
}