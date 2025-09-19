#include "../include/linked_list.h"
#include "../../debug/include/debug.h"
#include <stdlib.h>

void list_free(struct linked_list* ll, void* userdata, void (*deallocator) (void* data, void* userdata))
{
    struct list_item* curr = ll->head;
    while (curr)
    {
        struct list_item* del_item = curr;
        curr = del_item->next;
        if (deallocator)
            deallocator(del_item->data, userdata);
        free(del_item);
    }
    ll->head = NULL;
    ll->size = 0;
}