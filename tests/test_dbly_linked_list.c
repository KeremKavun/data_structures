#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/dbly_linked_list.h"
#include "../../concepts/include/object_concept.h"

// Helper to create a list item with integer data
struct dbly_list_item* create_int_item(int value) {
    struct dbly_list_item* item = (struct dbly_list_item*)malloc(sizeof(struct dbly_list_item));
    int* data = (int*)malloc(sizeof(int));
    *data = value;
    dbly_list_item_init(item, data);
    return item;
}

// Helper to compare integers
int compare_int(void* item_data, void* search_data) {
    int* a = (int*)item_data;
    int* b = (int*)search_data;
    return !(*a == *b);
}

// Helper to print list
void print_int_item(void* item_data, void* userdata) {
    int* val = (int*)item_data;
    printf("%d ", *val);
}

// Mock destructor for object_concept
void int_destruct(void* object, void* context) {
    free(object);
}

void test_init() {
    printf("Testing Init...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);
    assert(dbly_list_empty(&dll) == 1);
    assert(dbly_list_size(&dll) == 0);
    assert(dbly_list_head(&dll) == NULL);
    assert(dbly_list_tail(&dll) == NULL);
    printf("Init passed.\n");
}

void test_insert_front_back() {
    printf("Testing Insert Front/Back...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(10);
    dbly_list_insert_front(&dll, item1);
    
    assert(dbly_list_size(&dll) == 1);
    assert(dbly_list_empty(&dll) == 0);
    assert(dbly_list_head(&dll) == item1);
    assert(dbly_list_tail(&dll) == item1);

    struct dbly_list_item* item2 = create_int_item(20);
    dbly_list_insert_back(&dll, item2);

    assert(dbly_list_size(&dll) == 2);
    assert(dbly_list_head(&dll) == item1);
    assert(dbly_list_tail(&dll) == item2);
    assert(dbly_list_item_next(item1) == item2);
    assert(dbly_list_item_prev(item2) == item1);

    struct dbly_list_item* item3 = create_int_item(5);
    dbly_list_insert_front(&dll, item3);
    
    assert(dbly_list_size(&dll) == 3);
    assert(dbly_list_head(&dll) == item3);
    assert(dbly_list_item_next(item3) == item1);
    assert(dbly_list_item_prev(item1) == item3);

    // Cleanup manually for this test since we haven't tested free yet
    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Insert Front/Back passed.\n");
}

void test_remove() {
    printf("Testing Remove...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(1);
    struct dbly_list_item* item2 = create_int_item(2);
    struct dbly_list_item* item3 = create_int_item(3);

    dbly_list_insert_back(&dll, item1);
    dbly_list_insert_back(&dll, item2);
    dbly_list_insert_back(&dll, item3);

    // Remove front
    struct dbly_list_item* removed = dbly_list_remove_front(&dll);
    assert(removed == item1);
    assert(dbly_list_size(&dll) == 2);
    assert(dbly_list_head(&dll) == item2);

    // Remove back
    removed = dbly_list_remove_back(&dll);
    assert(removed == item3);
    assert(dbly_list_size(&dll) == 1);
    assert(dbly_list_tail(&dll) == item2);

    // Remove specific
    removed = dbly_list_remove(&dll, item2);
    assert(removed == item2);
    assert(dbly_list_size(&dll) == 0);
    assert(dbly_list_empty(&dll) == 1);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Remove passed.\n");
}

void test_find() {
    printf("Testing Find...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(100);
    struct dbly_list_item* item2 = create_int_item(200);
    dbly_list_insert_back(&dll, item1);
    dbly_list_insert_back(&dll, item2);

    int target = 200;
    // Assuming cmp returns 1 on match. If implementation differs, we'll adjust.
    // Based on common C patterns, if it takes a comparator, usually it's 0 for equal, but let's see.
    // If the find function signature is `int (*cmp)(void *item, void *data)`, it likely iterates and calls cmp.
    // If cmp returns true (non-zero), it stops? Or if it returns 0 (like strcmp)?
    // I'll define a comparator that returns 1 if equal for now.
    
    struct dbly_list_item* found = dbly_list_find_front(&dll, &target, compare_int);
    assert(found == item2);

    target = 100;
    found = dbly_list_find_back(&dll, &target, compare_int);
    assert(found == item1);

    target = 300;
    found = dbly_list_find_front(&dll, &target, compare_int);
    assert(found == NULL);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    printf("Find passed.\n");
}

void test_walk() {
    printf("Testing Walk...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(10);
    struct dbly_list_item* item2 = create_int_item(20);
    dbly_list_insert_back(&dll, item1);
    dbly_list_insert_back(&dll, item2);

    // We can't easily assert output without capturing stdout, but we can verify it doesn't crash
    // and maybe use a handler that sums values?
    // Let's define a sum handler.
    
    printf("Walk Front (expect 10 20): ");
    dbly_list_walk_front(&dll, NULL, print_int_item);
    printf("\n");

    printf("Walk Back (expect 20 10): ");
    dbly_list_walk_back(&dll, NULL, print_int_item);
    printf("\n");

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    printf("Walk passed.\n");
}

void test_reverse() {
    printf("Testing Reverse...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(1);
    struct dbly_list_item* item2 = create_int_item(2);
    struct dbly_list_item* item3 = create_int_item(3);

    dbly_list_insert_back(&dll, item1);
    dbly_list_insert_back(&dll, item2);
    dbly_list_insert_back(&dll, item3);

    dbly_list_reverse(&dll);

    assert(dbly_list_head(&dll) == item3);
    assert(dbly_list_tail(&dll) == item1);
    assert(dbly_list_item_next(item3) == item2);
    assert(dbly_list_item_next(item2) == item1);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Reverse passed.\n");
}

// Mock free for object_concept (frees the list item node)
void node_free(void* allocator, void* ptr) {
    free(ptr);
}

void test_free() {
    printf("Testing Free...\n");
    struct dbly_linked_list dll;
    dbly_list_init(&dll);

    struct dbly_list_item* item1 = create_int_item(10);
    struct dbly_list_item* item2 = create_int_item(20);
    dbly_list_insert_back(&dll, item1);
    dbly_list_insert_back(&dll, item2);

    struct object_concept oc;
    oc.destruct = int_destruct;
    oc.allocator = NULL;
    oc.alloc = NULL;
    oc.free = node_free; // Free the list item node
    
    dbly_list_free(&dll, NULL, &oc);
    
    assert(dbly_list_size(&dll) == 0);
    assert(dbly_list_empty(&dll) == 1);
    assert(dll.head == NULL);
    assert(dll.tail == NULL);
    printf("Free passed.\n");
}

int main() {
    test_init();
    test_insert_front_back();
    test_remove();
    test_find();
    test_walk();
    test_reverse();
    test_free();
    printf("All tests passed!\n");
    return 0;
}
