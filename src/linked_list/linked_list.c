#include "../../include/linked_list.h"
#include "../../../debug/include/debug.h"

// *** list_item implementation *** //

void list_item_init(struct list_item* li, void* data)
{
    li->next = NULL;
    li->data = data;
}

struct list_item** list_item_next(struct list_item* li)
{
    return &li->next;
}

void* list_item_data(struct list_item* li)
{
    return li->data;
}

// *** linked_list implementation *** //

void list_init(struct linked_list* ll)
{
    ll->head = NULL;
    ll->size = 0;
}

void list_insert(struct linked_list* ll, struct list_item** pos, struct list_item* new_item)
{
    struct list_item* curr_item = *pos;
    *pos = new_item;
    new_item->next = curr_item;
    ll->size++;
}

void list_insert_front(struct linked_list* ll, struct list_item* new_item)
{
    struct list_item** head = list_head(ll);
    new_item->next = *head;
    *head = new_item;
    ll->size++;
}

void list_insert_back(struct linked_list* ll, struct list_item* new_item)
{
    struct list_item** tail = list_tail(ll);
    if (*tail)
        (*tail)->next = new_item;
    else
        *tail = new_item;
    ll->size++;
}

struct list_item* list_remove(struct linked_list* ll, struct list_item** li)
{
    struct list_item* del = *li;
    if (!del)
        return NULL;
    *li = del->next;
    del->next = NULL;
    ll->size--;
    return del;
}

struct list_item* list_remove_front(struct linked_list* ll)
{
    return list_remove(ll, list_head(ll));
}

struct list_item* list_remove_back(struct linked_list* ll)
{
    return list_remove(ll, list_tail(ll));
}

struct list_item** list_head(struct linked_list* ll)
{
    return &ll->head;
}

struct list_item** list_tail(struct linked_list* ll)
{
    struct list_item** curr = &ll->head;
    while (*curr && (*curr)->next)
        curr = &(*curr)->next;
    return curr;
}

struct list_item** list_find(struct linked_list* ll, void* data, int (*cmp) (struct list_item* item, void* data))
{
    struct list_item** target = &ll->head;
    while (*target && cmp(*target, data) != 0)
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

void list_walk(struct linked_list* ll, void* userdata, void (*handler) (struct list_item* item, void* userdata))
{
    struct list_item** item = &ll->head;
    while (*item)
    {
        handler(*item, userdata);
        item = &(*item)->next;
    }
}

void list_reverse(struct linked_list* ll)
{
    struct list_item* prev = NULL;
    struct list_item* curr = ll->head;
    while (curr)
    {
        struct list_item* next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    ll->head = prev; // Update head to new first element
}