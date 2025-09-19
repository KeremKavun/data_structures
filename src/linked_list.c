#include "../include/linked_list.h"
#include "../../debug/include/debug.h"

void list_init(struct linked_list* ll)
{
    ll->head = NULL;
    ll->size = 0;
}

void list_insert(struct linked_list* ll, struct list_item* pos, struct list_item* new_item)
{
    new_item->next = pos->next;
    pos->next = new_item;
    ll->size++;
}

void list_remove(struct linked_list* ll, struct list_item* li)
{
    struct list_item** target = &ll->head;
    while (*target != li)
        target = &(*target)->next;
    *target = (*target)->next; // same with li->next
    ll->size--;
}

struct list_item* list_find(struct linked_list* ll, void* data, int (*cmp) (const void* data, const void* li_data))
{
    struct list_item* target = ll->head;
    while (cmp(data, target->data) != 0)
        target = target->next;
    return target;
}