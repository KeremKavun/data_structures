#include "../include/list_item.h"
#include <stdlib.h>

void list_item_init(struct list_item* li, void* data)
{
    li->next = NULL;
    li->data = data;
}