#include <ds/queue/lqueue.h>
#include <ds/linkedlists/slist.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct queue_item {
    void                    *data;
    struct slist_item       hook;
};

struct lqueue {
    struct slist                    contents;
    struct slist_item               **rear;
    struct allocator_concept        ac;
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
    slist_init(&lq->contents);
    lq->rear = slist_head(&lq->contents);
    lq->ac = *ac;
    return lq;
}

void lqueue_destroy(struct lqueue *lq, deinit_cb deinit)
{
    assert(lq != NULL);
    void *data;
    while ((data = ldequeue(lq))) {
        if (deinit)
            deinit(data);
    }
    free(lq);
}

size_t lqueue_node_sizeof()
{
    return sizeof(struct queue_item);
}

/* =========================================================================
 * Enqueue & Dequeue
 * ========================================================================= */

int lenqueue(struct lqueue *lq, void *new_item)
{
    assert(lq != NULL);
    struct queue_item *qui = lq->ac.alloc(lq->ac.allocator);
    if (!qui)
        return 1;
    qui->data = new_item;
    slist_insert(&lq->contents, lq->rear, &qui->hook);
    lq->rear = slist_item_next(lq->rear);
    return 0;
}

void *ldequeue(struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item **head = slist_head(&lq->contents);
    if (*head == NULL)
        return NULL;
    struct slist_item *to_remove = *head;
    slist_remove(&lq->contents, head);
    if (*head == NULL)
        lq->rear = slist_head(&lq->contents);
    struct queue_item *qui = slist_entry(to_remove, struct queue_item, hook);
    void *data = qui->data;
    lq->ac.free(lq->ac.allocator, qui);
    return data;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

void *lqueue_front(struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item *front = *slist_head(&lq->contents);
    return (front) ? slist_entry(front, struct queue_item, hook)->data : NULL;
}

void *lqueue_rear(struct lqueue *lq)
{
    assert(lq != NULL);
    struct slist_item *real_rear = (struct slist_item *) lq->rear;
    return (real_rear) ? slist_entry(real_rear, struct queue_item, hook)->data : NULL;
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
    slist_foreach(iter, slist_head(&lq->contents), NULL) {
        struct queue_item *parent = slist_entry(*iter, struct queue_item, hook);
        handler(parent->data, context);
    }
}