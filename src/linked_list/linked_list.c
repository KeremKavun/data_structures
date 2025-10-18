#include "../../include/linked_list.h"
#include "../../../debug/include/debug.h"

void list_item_init(struct list_item* li, void* data)
{
    li->data = data;
    li->next = NULL;
}

void list_init(struct linked_list* ll)
{
    ll->head = NULL;
    ll->size = 0;
}

void list_insert(struct linked_list* ll, struct list_item* pos, struct list_item* new_item)
{
    struct list_item** curr = &ll->head;
    while ((*curr)->next != pos)
        curr = &(*curr)->next;
    pos->next = (*curr)->next;
    (*curr)->next = new_item;
    ll->size++;
}

void list_insert_front(struct linked_list* ll, struct list_item* new_item)
{
    new_item->next = ll->head;
    ll->head = new_item;
    ll->size++;
}

void list_insert_back(struct linked_list* ll, struct list_item* new_item)
{
    struct list_item** curr = &ll->head;
    while (*curr)
        curr = &(*curr)->next;
    *curr = new_item;
    ll->size++;
}

struct list_item* list_remove(struct linked_list* ll, struct list_item** li)
{
    struct list_item* del = *li;
    *li = del->next;
    ll->size--;
    return del;
}

struct list_item* list_remove_front(struct linked_list* ll)
{
    return list_remove(ll, list_head(ll));
}

struct list_item* list_remove_back(struct linked_list* ll)
{
    list_remove(ll, list_tail(ll));
}

struct list_item** list_head(struct linked_list* ll)
{
    return &ll->head;
}

struct list_item** list_tail(struct linked_list* ll)
{
    struct list_item** curr = &ll->head;
    while (*curr != NULL && (*curr)->next != NULL)
        curr = &(*curr)->next;
    return curr;
}

struct list_item** list_find(struct linked_list* ll, void* data, int (*cmp) (struct list_item* item, void* data))
{
    struct list_item** target = &ll->head;
    while (*target && cmp(data, (*target)->data) != 0)
        target = &(*target)->next;
    return target;
}

int list_empty(struct linked_list* ll)
{
    return ll->head == NULL;
}

size_t list_size(struct linked_list* ll)
{
    return ll->size;
}

void list_walk(struct linked_list* ll, void* userdata, int (*handler) (struct list_item* item, void* userdata))
{
    struct list_item** item = &ll->head;
    while (*item)
    {
        handler(*item, userdata);
        item = &(*item)->next;
    }
}