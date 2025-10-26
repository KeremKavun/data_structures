#include "../include/lqueue.h"
#include "../../linked_lists/include/dbly_linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int lqueue_init(struct lqueue* lq)
{
    struct dbly_linked_list* contents = malloc(sizeof(struct dbly_linked_list));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }

    lq->contents = contents;
    dbly_list_init(lq->contents);
    return 0;
}

int lenqueue(struct lqueue* lq, void* new_item)
{
    struct dbly_list_item* new_node = malloc(sizeof(struct dbly_list_item));
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
    free(del);
    return data;
}

void* lqueue_peek(const struct lqueue* lq)
{
    struct dbly_list_item* peek = dbly_list_head(lq->contents);
    return peek ? peek->data : NULL;
}

int lqueue_empty(const struct lqueue* lq)
{
    return dbly_list_empty(lq->contents);
}

size_t lqueue_size(const struct lqueue* lq)
{
    return dbly_list_size(lq->contents);
}

void lqueue_walk(const struct lqueue* lq, void* userdata, void (*handler) (void* item, void* userdata))
{
    dbly_list_walk_front(lq->contents, userdata, handler);
}

void lqueue_free(struct lqueue* lq, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    dbly_list_free(lq->contents, userdata, deallocator);
    free(lq->contents);
}