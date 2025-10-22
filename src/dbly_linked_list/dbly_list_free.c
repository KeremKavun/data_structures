#include "../../include/dbly_linked_list.h"
#include "../../../debug/include/debug.h"
#include <stdlib.h>

void dbly_list_free(struct dbly_linked_list* dll, void* userdata, void (*deallocator) (void* data, void* userdata))
{
    struct dbly_list_item* curr = dll->head;
    while (curr)
    {
        struct dbly_list_item* del_item = curr;
        if (deallocator)
            deallocator(del_item->data, userdata);
        curr = del_item->next;
        free(del_item);
    }
    dll->head = dll->tail = NULL;
    dll->size = 0;
}