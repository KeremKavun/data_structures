#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ds/linkedlists/dlist.h>

/* * =================================================================================
 * 1. TEST DATA STRUCTURES
 * =================================================================================
 */

// Test data structure with embedded dlist_item
typedef struct {
    int id;
    struct dlist_item hook;  // Intrusive list node
} my_data_t;

// Counter for deinit callbacks
static int counter = 0;

/* * =================================================================================
 * 2. HELPER FUNCTIONS
 * =================================================================================
 */

// Deinit callback for cleanup test
void test_deinit_func(void *item) {
    (void) item;
    counter++;
    // In real usage, you might free the parent struct here
    // my_data_t *data = dlist_entry(item, my_data_t, hook);
    // free(data);
}

// Predicate to find entry with id == 2
int find_id_2(my_data_t *data) {
    return data->id == 2 ? 0 : 1;
}

/* * =================================================================================
 * 3. UNIT TESTS
 * =================================================================================
 */

void test_lifecycle_basic() {
    printf("[Test] Lifecycle & Basic Push/Pop... ");
    struct dlist list;
    
    dlist_init(&list);
    assert(dlist_empty(&list));
    assert(dlist_size(&list) == 0);

    // Create data with embedded hooks
    my_data_t d1 = {.id = 1};
    my_data_t d2 = {.id = 2};
    my_data_t d3 = {.id = 3};

    // Push Front: 2 -> 1
    dlist_push_front(&list, &d1.hook);
    dlist_push_front(&list, &d2.hook);
    
    // Check Head
    struct dlist_item *head = dlist_head(&list);
    my_data_t *head_data = dlist_entry(head, my_data_t, hook);
    assert(head_data->id == 2);

    // Push Back: 2 -> 1 -> 3
    dlist_push_back(&list, &d3.hook);
    struct dlist_item *tail = dlist_tail(&list);
    my_data_t *tail_data = dlist_entry(tail, my_data_t, hook);
    assert(tail_data->id == 3);
    assert(dlist_size(&list) == 3);

    // Remove Front (Remove 2) -> 1 -> 3
    struct dlist_item *removed = dlist_remove_front(&list);
    my_data_t *removed_data = dlist_entry(removed, my_data_t, hook);
    assert(removed_data->id == 2);
    head_data = dlist_entry(dlist_head(&list), my_data_t, hook);
    assert(head_data->id == 1);

    // Remove Back (Remove 3) -> 1
    removed = dlist_remove_back(&list);
    removed_data = dlist_entry(removed, my_data_t, hook);
    assert(removed_data->id == 3);
    tail_data = dlist_entry(dlist_tail(&list), my_data_t, hook);
    assert(tail_data->id == 1);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_forward_safe_iteration() {
    printf("[Test] Forward Safe Iteration & Removal... ");
    struct dlist list;
    dlist_init(&list);

    // Create array of data
    my_data_t data[5] = {
        {.id = 10}, {.id = 20}, {.id = 30}, {.id = 40}, {.id = 50}
    };
    
    // Create: 10 -> 20 -> 30 -> 40 -> 50
    for(int i = 0; i < 5; i++) {
        dlist_push_back(&list, &data[i].hook);
    }

    struct dlist_item *iter, *n;
    
    // Remove 20 and 40
    dlist_foreach_fr_safe(iter, n, dlist_head(&list), &list.sentinel) {
        my_data_t *val = dlist_entry(iter, my_data_t, hook);
        if (val->id == 20 || val->id == 40) {
            dlist_remove(&list, iter);
        }
    }

    // Verify: 10 -> 30 -> 50
    assert(dlist_size(&list) == 3);
    int expected[] = {10, 30, 50};
    int i = 0;
    dlist_foreach_fr(iter, dlist_head(&list), &list.sentinel) {
        my_data_t *val = dlist_entry(iter, my_data_t, hook);
        assert(val->id == expected[i++]);
    }

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_backward_iteration() {
    printf("[Test] Backward Iteration... ");
    struct dlist list;
    dlist_init(&list);

    my_data_t data[3] = {{.id = 1}, {.id = 2}, {.id = 3}};
    for(int i = 0; i < 3; i++) {
        dlist_push_back(&list, &data[i].hook);
    }

    struct dlist_item *iter;
    int expected[] = {3, 2, 1}; // Backwards
    int i = 0;
    
    // Use tail and sentinel to iterate backwards
    dlist_foreach_bk(iter, dlist_tail(&list), &list.sentinel) {
        my_data_t *val = dlist_entry(iter, my_data_t, hook);
        assert(val->id == expected[i++]);
    }
    assert(i == 3);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_find_macros() {
    printf("[Test] Find Macros (Fr & Bk)... ");
    struct dlist list;
    dlist_init(&list);

    my_data_t data[5] = {
        {.id = 1}, {.id = 2}, {.id = 3}, {.id = 2}, {.id = 4}
    };
    for(int i = 0; i < 5; i++) {
        dlist_push_back(&list, &data[i].hook);
    }

    my_data_t *res;

    // Find Forward (First 2)
    dlist_find_entry_fr(res, dlist_head(&list), &list.sentinel, hook, find_id_2);
    assert(res != NULL);
    assert(res->id == 2);
    // Should be data[1] (second element)
    assert(res == &data[1]);

    // Find Backward (Last 2)
    dlist_find_entry_bk(res, dlist_tail(&list), &list.sentinel, hook, find_id_2);
    assert(res != NULL);
    assert(res->id == 2);
    // Should be data[3] (fourth element)
    assert(res == &data[3]);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_reverse_list() {
    printf("[Test] List Reversal... ");
    struct dlist list;
    dlist_init(&list);

    my_data_t data[3] = {{.id = 1}, {.id = 2}, {.id = 3}};
    for(int i = 0; i < 3; i++) {
        dlist_push_back(&list, &data[i].hook);
    }

    dlist_reverse(&list);

    // Expect 3 -> 2 -> 1
    struct dlist_item *iter = dlist_head(&list);
    my_data_t *val = dlist_entry(iter, my_data_t, hook);
    assert(val->id == 3);
    
    iter = dlist_item_next(iter);
    val = dlist_entry(iter, my_data_t, hook);
    assert(val->id == 2);
    
    iter = dlist_item_next(iter);
    val = dlist_entry(iter, my_data_t, hook);
    assert(val->id == 1);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_cleanup() {
    printf("[Test] Cleanup with Deinit Callback... ");
    struct dlist list;
    dlist_init(&list);
    
    my_data_t data[2] = {{.id = 1}, {.id = 2}};
    dlist_push_back(&list, &data[0].hook);
    dlist_push_back(&list, &data[1].hook);

    // Reset counter and deinit with callback
    counter = 0;
    dlist_deinit(&list, test_deinit_func);

    assert(dlist_size(&list) == 0);
    assert(counter == 2); // Deinit called twice
    printf("PASSED\n");
}

void test_insert_operations() {
    printf("[Test] Insert After/Before... ");
    struct dlist list;
    dlist_init(&list);

    my_data_t d1 = {.id = 1};
    my_data_t d2 = {.id = 2};
    my_data_t d3 = {.id = 3};
    my_data_t d4 = {.id = 4};

    // Start with: 1 -> 3
    dlist_push_back(&list, &d1.hook);
    dlist_push_back(&list, &d3.hook);

    // Insert 2 after 1: 1 -> 2 -> 3
    dlist_insert_after(&list, &d1.hook, &d2.hook);
    
    // Insert 4 before 3: 1 -> 2 -> 4 -> 3
    dlist_insert_before(&list, &d3.hook, &d4.hook);

    // Verify order
    assert(dlist_size(&list) == 4);
    int expected[] = {1, 2, 4, 3};
    int i = 0;
    struct dlist_item *iter;
    dlist_foreach_fr(iter, dlist_head(&list), &list.sentinel) {
        my_data_t *val = dlist_entry(iter, my_data_t, hook);
        assert(val->id == expected[i++]);
    }

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_empty_list_operations() {
    printf("[Test] Empty List Edge Cases... ");
    struct dlist list;
    dlist_init(&list);

    // Test removal from empty list
    struct dlist_item *removed = dlist_remove_front(&list);
    assert(removed == NULL);
    
    removed = dlist_remove_back(&list);
    assert(removed == NULL);

    // Test head/tail on empty list (should return sentinel)
    assert(dlist_head(&list) == &list.sentinel);
    assert(dlist_tail(&list) == &list.sentinel);

    dlist_deinit(&list, NULL);
    printf("PASSED\n");
}

int main() {
    printf("=== DList Tests (Intrusive Version) ===\n");
    test_lifecycle_basic();
    test_forward_safe_iteration();
    test_backward_iteration();
    test_find_macros();
    test_reverse_list();
    test_cleanup();
    test_insert_operations();
    test_empty_list_operations();
    printf("=== All Tests Passed ===\n");
    return 0;
}
