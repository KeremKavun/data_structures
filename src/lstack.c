#include "../include/lstack.h"
#include "../../linked_lists/include/dbly_linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int lstack_init(struct lstack* ls)
{
    struct dbly_linked_list* contents = malloc(sizeof(struct dbly_linked_list));
    if (!contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }

    dbly_list_init(ls->contents);
    return 0;
}

int lpush(struct lstack* ls, void* new_item)
{
    struct dbly_list_item* new_node = malloc(sizeof(struct dbly_list_item));
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }

    dbly_list_item_init(new_node, new_item);
    dbly_list_insert_back(ls->contents, new_node);
    return 0;
}

void* lpop(struct lstack* ls)
{
    struct dbly_list_item* del = dbly_list_remove_back(ls->contents);
    if (!del)
        return NULL;
    void* data = del->data;
    free(del);
    return data;
}

void* ltop(struct lstack* ls)
{
    struct dbly_list_item* peek = dbly_list_tail(ls->contents);
    return peek ? peek->data : NULL;
}

int lstack_empty(const struct lstack* ls)
{
    return dbly_list_empty(ls->contents);
}

size_t lstack_size(const struct lstack* ls)
{
    return dbly_list_size(ls->contents);
}

void lstack_walk(struct lstack* ls, void* userdata, void (*handler) (void* item, void* userdata))
{
    dbly_list_walk_back(ls->contents, userdata, handler);
}

void lstack_free(struct lstack* ls, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    void* data;
    while ((data = lpop(ls)))
        deallocator(data, userdata);
    free(ls->contents);
    ls->contents->head = ls->contents->tail = NULL;
    ls->contents->size = 0;
}