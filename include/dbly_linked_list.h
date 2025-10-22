#ifndef DBLY_LINKED_LIST_H
#define DBLY_LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// dbly_list_item struct
struct dbly_list_item
{
    struct dbly_list_item* prev;
    struct dbly_list_item* next;
    void* data;
};

struct dbly_linked_list
{
    struct dbly_list_item* head;
    struct dbly_list_item* tail;
    size_t size;
};

// Init dbly_list_item
void dbly_list_item_init(struct dbly_list_item* dli, void* data);
// Get pointer that points to previous element, which can be used for insert or remove methods
struct dbly_list_item* dbly_list_item_prev(struct dbly_list_item* dli);
// Get pointer that points to next element, which can be used for insert or remove methods
struct dbly_list_item* dbly_list_item_next(struct dbly_list_item* dli);
// Get the data from list_item
void* dbly_list_item_data(struct dbly_list_item* dli);

// Init linked_list
void dbly_list_init(struct dbly_linked_list* dll);
// Insert a dbly_list_item you own(!!) at pos
void dbly_list_insert(struct dbly_linked_list* dll, struct dbly_list_item* pos, struct dbly_list_item* new_item);
// Insert an item at front
void dbly_list_insert_front(struct dbly_linked_list* dll, struct dbly_list_item* new_item);
// Isert an item at back
void dbly_list_insert_back(struct dbly_linked_list* dll, struct dbly_list_item* new_item);
// Insert a dbly_list_item you own(!!) after pos
struct dbly_list_item* dbly_list_remove(struct dbly_linked_list* dll, struct dbly_list_item* dli);
// Remove front
struct dbly_list_item* dbly_list_remove_front(struct dbly_linked_list* dll);
// Remove back
struct dbly_list_item* dbly_list_remove_back(struct dbly_linked_list* dll);
// Get head
struct dbly_list_item* dbly_list_head(struct dbly_linked_list* dll);
// Get tail (returns address of the next pointer of the last node)
struct dbly_list_item* dbly_list_tail(struct dbly_linked_list* dll);
// Find the first dbly_list_item with the specific data, beginning to search at the head
struct dbly_list_item* dbly_list_find_front(struct dbly_linked_list* dll, void* userdata, int (*cmp) (struct dbly_list_item* item, void* data));
// Find the first dbly_list_item with the specific data, beginning to search at the tail
struct dbly_list_item* dbly_list_find_back(struct dbly_linked_list* dll, void* userdata, int (*cmp) (struct dbly_list_item* item, void* data));
// Returns 1 if the list is empty, 0 otherwise
int dbly_list_empty(struct dbly_linked_list* dll);
// Returns size of the list
size_t dbly_list_size(struct dbly_linked_list* dll);
// Traverses the list, beginning at the head
void dbly_list_walk_front(struct dbly_linked_list* dll, void* userdata, void (*handler) (struct dbly_list_item* item, void* userdata));
// Traverses the list, beginning from the tail
void dbly_list_walk_back(struct dbly_linked_list* dll, void* userdata, void (*handler) (struct dbly_list_item* item, void* userdata));
// Reverses the list
void dbly_list_reverse(struct dbly_linked_list* dll);
// Free the structure of the linked_list (not linked_list itself). Be careful about object pointers stored in the linked_list and
// where the object allocated (stack or heap?). Deallocator runs on every data member of the dbly_list_item
void dbly_list_free(struct dbly_linked_list* dll, void* userdata, void (*deallocator) (void* data, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // DBLY_LINKED_LIST_H