#include "../include/dbly_linked_list.h"
#include <stdlib.h>

static struct dbly_list_item* dbly_list_find_helper
(
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    int (*cmp) (void* item, void* data)
);
static void dbly_list_walk_helper
(
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    void (*data_handler) (void* data, void* userdata),
    void* context,
    void (*item_handler) (void* item, void* context)
);
static void exchange_ptrs(void* context, void* item);

// *** dbly_list_item implementation *** //

void dbly_list_item_init(struct dbly_list_item* dli, void* data)
{
    dli->prev = NULL;
    dli->next = NULL;
    dli->data = data;
}

struct dbly_list_item* dbly_list_item_prev(struct dbly_list_item* dli)
{
    return dli->prev;
}

struct dbly_list_item* dbly_list_item_next(struct dbly_list_item* dli)
{
    return dli->next;
}

void* dbly_list_item_data(struct dbly_list_item* dli)
{
    return dli->data;
}

// *** dbly_linked_list implementation *** //

void dbly_list_init(struct dbly_linked_list* dll)
{
    dll->head = NULL;
    dll->tail = NULL;
    dll->size = 0;
}

void dbly_list_insert(struct dbly_linked_list* dll, struct dbly_list_item* pos, struct dbly_list_item* new_item)
{
    if (pos == dll->head)
        dbly_list_insert_front(dll, new_item);
    else if (pos == NULL)
        dbly_list_insert_back(dll, new_item);
    else
    {
        new_item->next = pos;
        new_item->prev = pos->prev;

        if (pos->prev)
            pos->prev->next = new_item;
        pos->prev = new_item;
        dll->size++;
    }
}

void dbly_list_insert_front(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    new_item->prev = NULL;

    if (dll->head)
    {
        dll->head->prev = new_item;
        new_item->next = dll->head;
    }
    else
    {
        dll->tail = new_item;
        new_item->next = NULL;
    }
    dll->head = new_item;
    dll->size++;
}

void dbly_list_insert_back(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    new_item->next = NULL;

    if (dll->tail)
    {
        dll->tail->next = new_item;
        new_item->prev = dll->tail;
    }
    else
    {
        dll->head = new_item;
        new_item->prev = NULL;
    }
    dll->tail = new_item;
    dll->size++;
}

struct dbly_list_item* dbly_list_remove(struct dbly_linked_list* dll, struct dbly_list_item* dli)
{
    struct dbly_list_item* del = dli;

    if (dli->prev)
        dli->prev->next = del->next;
    else
        dll->head = del->next;

    if (dli->next)
        dli->next->prev = del->prev;
    else
        dll->tail = del->prev;

    del->prev = del->next = NULL;
    dll->size--;
    return del;
}

struct dbly_list_item* dbly_list_remove_front(struct dbly_linked_list* dll)
{
    if (!dll->head)
        return NULL;
    return dbly_list_remove(dll, dll->head);
}

struct dbly_list_item* dbly_list_remove_back(struct dbly_linked_list* dll)
{
    if (!dll->tail)
        return NULL;
    return dbly_list_remove(dll, dll->tail);
}

struct dbly_list_item* dbly_list_head(struct dbly_linked_list* dll)
{
    return dll->head;
}

struct dbly_list_item* dbly_list_tail(struct dbly_linked_list* dll)
{
    return dll->tail;
}

struct dbly_list_item* dbly_list_find_front(struct dbly_linked_list* dll, void* userdata, int (*cmp) (void* item, void* data))
{
    return dbly_list_find_helper(dbly_list_head(dll), dbly_list_item_next, userdata, cmp);
}

struct dbly_list_item* dbly_list_find_back(struct dbly_linked_list* dll, void* userdata, int (*cmp) (void* item, void* data))
{
    return dbly_list_find_helper(dbly_list_tail(dll), dbly_list_item_prev, userdata, cmp);
}

int dbly_list_empty(const struct dbly_linked_list* dll)
{
    return (dll->size == 0);
}

size_t dbly_list_size(const struct dbly_linked_list* dll)
{
    return dll->size;
}

void dbly_list_walk_front(struct dbly_linked_list* dll, void* userdata, void (*handler) (void* item, void* userdata))
{
    dbly_list_walk_helper(dbly_list_head(dll), dbly_list_item_next, userdata, handler, NULL, NULL);
}

void dbly_list_walk_back(struct dbly_linked_list* dll, void* userdata, void (*handler) (void* item, void* userdata))
{
    dbly_list_walk_helper(dbly_list_tail(dll), dbly_list_item_prev, userdata, handler, NULL, NULL);
}

void dbly_list_reverse(struct dbly_linked_list* dll)
{
    dbly_list_walk_helper(dbly_list_head(dll), dbly_list_item_next, NULL, NULL, NULL, exchange_ptrs);
    struct dbly_list_item* temp = dll->head;
    dll->head = dll->tail;
    dll->tail = temp;
}

void dbly_list_free(struct dbly_linked_list *dll, void *context, struct object_concept *oc, struct allocator_concept* ac)
{
    dbly_list_walk_helper(dbly_list_head(dll), dbly_list_item_next, context, oc->deinit, ac->allocator, ac->free);
    dll->head = dll->tail = NULL;
    dll->size = 0;
}

// *** Helper functions *** //

static struct dbly_list_item* dbly_list_find_helper
(
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* data,
    int (*cmp) (void* item, void* data)
)
{
    while (entry)
    {
        if (cmp(entry->data, data) == 0)
            return entry;
        entry = it(entry);
    }
    return NULL;
}

static void dbly_list_walk_helper
(
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    void (*data_handler) (void* data, void* userdata),
    void* context,
    void (*item_handler) (void* item, void* context)
)
{
    while (entry)
    {
        struct dbly_list_item* old_item = entry;
        void* old_data = entry->data;
        entry = it(entry);
        if (data_handler)
            data_handler(old_data, userdata);
        if (item_handler)
            item_handler(context, old_item);
    }
}

static void exchange_ptrs(void* context, void* item)
{
    (void) context;
    struct dbly_list_item* nitem = item;
    struct dbly_list_item* next = nitem->next;
    nitem->next = nitem->prev;
    nitem->prev = next;
}