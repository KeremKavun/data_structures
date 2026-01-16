#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include <ds/linkedlists/dlist.h>

/* * =================================================================================
 * 1. DEPENDENCY MOCKS
 * (We mock Allocator/Object concepts because they are external dependencies)
 * =================================================================================
 */

// --- Object Concept Deinit Mock ---
// context is used as a counter to verify calls

static int counter = 0;

void test_deinit_func(void *object) {
    (void)object; 
    counter++;
}

// --- Test Data Helper ---
typedef struct { int id; } my_data_t;

int find_id_2(void *data) { 
    return ((my_data_t*)data)->id == 2 ? 0 : 1; 
}

/* * =================================================================================
 * 2. UNIT TESTS
 * =================================================================================
 */

void test_lifecycle_basic() {
    printf("[Test] Lifecycle & Basic Push/Pop... ");
    struct dlist list;
    
    // Setup Allocator for dlist_item
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    
    dlist_init(&list, &ac);
    assert(dlist_empty(&list));
    assert(dlist_size(&list) == 0);

    my_data_t d1={1}, d2={2}, d3={3};

    // Push Front: 2 -> 1
    dlist_push_front(&list, &d1);
    dlist_push_front(&list, &d2);
    
    // Check Head
    struct dlist_item *head = dlist_head(&list);
    assert(((my_data_t*)dlist_item_data(head))->id == 2);

    // Push Back: 2 -> 1 -> 3
    dlist_push_back(&list, &d3);
    struct dlist_item *tail = dlist_tail(&list);
    assert(((my_data_t*)dlist_item_data(tail))->id == 3);
    assert(dlist_size(&list) == 3);

    // Remove Front (Remove 2) -> 1 -> 3
    void *removed_data = dlist_remove_front(&list);
    assert(((my_data_t*)removed_data)->id == 2);
    assert(((my_data_t*)dlist_item_data(dlist_head(&list)))->id == 1);

    // Remove Back (Remove 3) -> 1
    removed_data = dlist_remove_back(&list);
    assert(((my_data_t*)removed_data)->id == 3);
    assert(((my_data_t*)dlist_item_data(dlist_tail(&list)))->id == 1);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_forward_safe_iteration() {
    printf("[Test] Forward Safe Iteration & Removal... ");
    struct dlist list;
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    dlist_init(&list, &ac);

    int vals[] = {10, 20, 30, 40, 50};
    // Create: 10 -> 20 -> 30 -> 40 -> 50
    for(int i=0; i<5; i++) dlist_push_back(&list, &vals[i]);

    struct dlist_item *iter, *n;
    struct dlist_item *end_marker = &list.sentinel; // Assuming circular sentinel implementation
    
    // Remove 20 and 40
    dlist_foreach_fr_safe(iter, n, dlist_head(&list), end_marker) {
        int *val = (int*)dlist_item_data(iter);
        if (*val == 20 || *val == 40) {
            dlist_remove(&list, iter);
        }
    }

    // Verify: 10 -> 30 -> 50
    assert(dlist_size(&list) == 3);
    int expected[] = {10, 30, 50};
    int i = 0;
    dlist_foreach_fr(iter, dlist_head(&list), end_marker) {
        assert(*(int*)dlist_item_data(iter) == expected[i++]);
    }

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_backward_iteration() {
    printf("[Test] Backward Iteration... ");
    struct dlist list;
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    dlist_init(&list, &ac);

    int vals[] = {1, 2, 3};
    for(int i=0; i<3; i++) dlist_push_back(&list, &vals[i]);

    struct dlist_item *iter;
    int expected[] = {3, 2, 1}; // Backwards
    int i = 0;
    
    // Use tail and sentinel to iterate backwards
    dlist_foreach_bk(iter, dlist_tail(&list), &list.sentinel) {
        assert(*(int*)dlist_item_data(iter) == expected[i++]);
    }
    assert(i == 3);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_find_macros() {
    printf("[Test] Find Macros (Fr & Bk)... ");
    struct dlist list;
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    dlist_init(&list, &ac);

    my_data_t d[] = {{1}, {2}, {3}, {2}, {4}}; // Note two 2s
    for(int i=0; i<5; i++) dlist_push_back(&list, &d[i]);

    struct dlist_item *res;

    // Find Forward (First 2)
    dlist_find_entry_fr(res, dlist_head(&list), &list.sentinel, find_id_2);
    assert(res != NULL);
    // Should be the 2nd item in list
    assert(res == dlist_head(&list)->next); 

    // Find Backward (Last 2)
    dlist_find_entry_bk(res, dlist_tail(&list), &list.sentinel, find_id_2);
    assert(res != NULL);
    // Should be the 4th item (2nd to last)
    assert(res == dlist_tail(&list)->prev); 

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_reverse_list() {
    printf("[Test] List Reversal... ");
    struct dlist list;
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    dlist_init(&list, &ac);

    int vals[] = {1, 2, 3};
    for(int i=0; i<3; i++) dlist_push_back(&list, &vals[i]);

    dlist_reverse(&list);

    // Expect 3 -> 2 -> 1
    struct dlist_item *iter = dlist_head(&list);
    assert(*(int*)dlist_item_data(iter) == 3);
    iter = dlist_item_next(iter);
    assert(*(int*)dlist_item_data(iter) == 2);
    iter = dlist_item_next(iter);
    assert(*(int*)dlist_item_data(iter) == 1);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_cleanup() {
    printf("[Test] Cleanup with Object Concept... ");
    struct dlist list;
    struct syspool pool = { .obj_size = sizeof(struct dlist_item) };
    struct allocator_concept ac = { .allocator = &pool, .alloc = sysalloc, .free = sysfree };
    struct object_concept oc = { .deinit = test_deinit_func };
    
    dlist_init(&list, &ac);
    int vals[] = {1, 2};
    dlist_push_back(&list, &vals[0]);
    dlist_push_back(&list, &vals[1]);

    // Pass count as context to verify callbacks
    counter = 0;
    dlist_deinit(&list, &oc);

    assert(dlist_size(&list) == 0);
    assert(counter == 2); // Deinit called twice
    printf("PASSED\n");
}

int main() {
    printf("=== DList Tests ===\n");
    test_lifecycle_basic();
    test_forward_safe_iteration();
    test_backward_iteration();
    test_find_macros();
    test_reverse_list();
    test_cleanup();
    printf("=== All Tests Passed ===\n");
    return 0;
}
