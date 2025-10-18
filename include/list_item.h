#ifndef LIST_ITEM_H
#define LIST_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

// list_item struct
struct list_item
{
    struct list_item* next;
    void* data;
};

// Init list_item
void list_item_init(struct list_item* li, void* data);

#ifdef __cplusplus
}
#endif

#endif // LIST_ITEM_H