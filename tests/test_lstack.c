#include <ds/stack/lstack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test data structure */
typedef struct {
    int id;
    char name[32];
} test_data_t;

/* Object concept implementation for test_data_t */
static void test_data_deinit(void* obj) {
    free(obj);
}

static struct object_concept test_oc = {
    .deinit = test_data_deinit
};

/* Walk handler for testing */
static void print_handler(void* item, void* context) {
    int* count = (int*)context;
    test_data_t* data = (test_data_t*)item;
    printf("  Item %d: id=%d, name=%s\n", *count, data->id, data->name);
    (*count)++;
}

/* Test helper to create test data */
static test_data_t* create_test_data(int id, const char* name) {
    test_data_t* data = (test_data_t*)malloc(sizeof(test_data_t));
    assert(data != NULL);
    data->id = id;
    strncpy(data->name, name, sizeof(data->name) - 1);
    data->name[sizeof(data->name) - 1] = '\0';
    return data;
}

/* Helper to create allocator concept */
static struct allocator_concept create_allocator(struct syspool* pool) {
    pool->obj_size = lstack_node_sizeof();
    struct allocator_concept ac = {
        .allocator = pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    return ac;
}

/* Test 1: Creation and Destruction */
static void test_create_destroy(void) {
    printf("\n=== Test 1: Creation and Destruction ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    
    assert(ls != NULL);
    assert(lstack_empty(ls) == 1);
    assert(lstack_size(ls) == 0);
    printf("✓ Stack created successfully\n");
    
    lstack_destroy(ls, test_oc.deinit);
    printf("✓ Stack destroyed successfully\n");
}

/* Test 2: Push and Pop Operations */
static void test_push_pop(void) {
    printf("\n=== Test 2: Push and Pop Operations ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    /* Push items */
    test_data_t* d1 = create_test_data(1, "First");
    test_data_t* d2 = create_test_data(2, "Second");
    test_data_t* d3 = create_test_data(3, "Third");
    
    assert(lpush(ls, d1) == 0);
    printf("✓ Pushed item 1\n");
    assert(lstack_size(ls) == 1);
    
    assert(lpush(ls, d2) == 0);
    printf("✓ Pushed item 2\n");
    assert(lstack_size(ls) == 2);
    
    assert(lpush(ls, d3) == 0);
    printf("✓ Pushed item 3\n");
    assert(lstack_size(ls) == 3);
    assert(lstack_empty(ls) == 0);
    
    /* Pop items (LIFO order) */
    test_data_t* popped = (test_data_t*)lpop(ls);
    assert(popped != NULL);
    assert(popped->id == 3);
    printf("✓ Popped item 3: %s\n", popped->name);
    free(popped);
    
    popped = (test_data_t*)lpop(ls);
    assert(popped != NULL);
    assert(popped->id == 2);
    printf("✓ Popped item 2: %s\n", popped->name);
    free(popped);
    
    popped = (test_data_t*)lpop(ls);
    assert(popped != NULL);
    assert(popped->id == 1);
    printf("✓ Popped item 1: %s\n", popped->name);
    free(popped);
    
    assert(lstack_empty(ls) == 1);
    assert(lstack_size(ls) == 0);
    printf("✓ Stack is empty after all pops\n");
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 3: Top Operation */
static void test_top(void) {
    printf("\n=== Test 3: Top Operation ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    test_data_t* d1 = create_test_data(100, "Top Item");
    test_data_t* d2 = create_test_data(200, "Bottom Item");
    
    lpush(ls, d2);
    lpush(ls, d1);
    
    test_data_t* top = (test_data_t*)ltop(ls);
    assert(top != NULL);
    assert(top->id == 100);
    printf("✓ Top item: id=%d, name=%s\n", top->id, top->name);
    
    /* Verify top doesn't remove item */
    assert(lstack_size(ls) == 2);
    printf("✓ Top operation doesn't modify stack size\n");
    
    /* Pop and verify top changes */
    test_data_t* popped = (test_data_t*)lpop(ls);
    free(popped);
    
    top = (test_data_t*)ltop(ls);
    assert(top != NULL);
    assert(top->id == 200);
    printf("✓ New top item after pop: id=%d, name=%s\n", top->id, top->name);
    
    /* Clean up */
    popped = (test_data_t*)lpop(ls);
    free(popped);
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 4: Empty Stack Operations */
static void test_empty_stack(void) {
    printf("\n=== Test 4: Empty Stack Operations ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    /* Pop from empty stack */
    void* result = lpop(ls);
    assert(result == NULL);
    printf("✓ Pop from empty stack returns NULL\n");
    
    /* Top from empty stack */
    result = ltop(ls);
    assert(result == NULL);
    printf("✓ Top from empty stack returns NULL\n");
    
    assert(lstack_empty(ls) == 1);
    assert(lstack_size(ls) == 0);
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 5: Walk Operation */
static void test_walk(void) {
    printf("\n=== Test 5: Walk Operation ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    /* Push multiple items */
    for (int i = 1; i <= 5; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Item-%d", i);
        test_data_t* data = create_test_data(i * 10, name);
        lpush(ls, data);
    }
    
    printf("Stack contents (top to bottom):\n");
    int count = 0;
    lstack_walk(ls, &count, print_handler);
    assert(count == 5);
    printf("✓ Walked through %d items\n", count);
    
    /* Clean up */
    while (!lstack_empty(ls)) {
        test_data_t* data = (test_data_t*)lpop(ls);
        free(data);
    }
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 6: Large Stack Stress Test */
static void test_stress(void) {
    printf("\n=== Test 6: Large Stack Stress Test ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    const int NUM_ITEMS = 1000;
    
    /* Push many items */
    for (int i = 0; i < NUM_ITEMS; i++) {
        test_data_t* data = create_test_data(i, "Stress");
        assert(lpush(ls, data) == 0);
    }
    
    assert(lstack_size(ls) == NUM_ITEMS);
    printf("✓ Pushed %d items\n", NUM_ITEMS);
    
    /* Pop all items and verify LIFO order */
    for (int i = NUM_ITEMS - 1; i >= 0; i--) {
        test_data_t* data = (test_data_t*)lpop(ls);
        assert(data != NULL);
        assert(data->id == i);
        free(data);
    }
    
    assert(lstack_empty(ls) == 1);
    printf("✓ Popped all %d items in correct LIFO order\n", NUM_ITEMS);
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 7: Destroy with Remaining Items */
static void test_destroy_with_items(void) {
    printf("\n=== Test 7: Destroy with Remaining Items ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    /* Push items without popping */
    for (int i = 0; i < 10; i++) {
        test_data_t* data = create_test_data(i, "Cleanup");
        lpush(ls, data);
    }
    
    printf("✓ Pushed 10 items\n");
    printf("✓ Destroying stack with remaining items\n");
    
    /* Destroy should clean up all remaining items */
    lstack_destroy(ls, test_oc.deinit);
    printf("✓ Stack destroyed successfully (items cleaned up)\n");
}

/* Test 8: Alternating Push/Pop */
static void test_alternating_ops(void) {
    printf("\n=== Test 8: Alternating Push/Pop Operations ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    for (int i = 0; i < 100; i++) {
        test_data_t* data = create_test_data(i, "Alt");
        lpush(ls, data);
        
        if (i % 2 == 1) {
            test_data_t* popped = (test_data_t*)lpop(ls);
            assert(popped->id == i);
            free(popped);
        }
    }
    
    /* Should have 50 items remaining (every even index) */
    assert(lstack_size(ls) == 50);
    printf("✓ Alternating push/pop: %zu items remaining\n", lstack_size(ls));
    
    /* Clean up */
    while (!lstack_empty(ls)) {
        test_data_t* data = (test_data_t*)lpop(ls);
        free(data);
    }
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 9: Multiple Stacks */
static void test_multiple_stacks(void) {
    printf("\n=== Test 9: Multiple Independent Stacks ===\n");
    
    struct syspool pool1 = {0}, pool2 = {0}, pool3 = {0};
    struct allocator_concept ac1 = create_allocator(&pool1);
    struct allocator_concept ac2 = create_allocator(&pool2);
    struct allocator_concept ac3 = create_allocator(&pool3);
    
    struct lstack* ls1 = lstack_create(&ac1);
    struct lstack* ls2 = lstack_create(&ac2);
    struct lstack* ls3 = lstack_create(&ac3);
    
    assert(ls1 != NULL && ls2 != NULL && ls3 != NULL);
    
    /* Push to different stacks */
    for (int i = 0; i < 5; i++) {
        lpush(ls1, create_test_data(i, "Stack1"));
        lpush(ls2, create_test_data(i + 100, "Stack2"));
        lpush(ls3, create_test_data(i + 200, "Stack3"));
    }
    
    assert(lstack_size(ls1) == 5);
    assert(lstack_size(ls2) == 5);
    assert(lstack_size(ls3) == 5);
    printf("✓ Created 3 independent stacks with 5 items each\n");
    
    /* Verify independence */
    test_data_t* top1 = (test_data_t*)ltop(ls1);
    test_data_t* top2 = (test_data_t*)ltop(ls2);
    test_data_t* top3 = (test_data_t*)ltop(ls3);
    
    assert(top1->id == 4);
    assert(top2->id == 104);
    assert(top3->id == 204);
    printf("✓ Each stack maintains independent data\n");
    
    /* Clean up all stacks */
    while (!lstack_empty(ls1)) free(lpop(ls1));
    while (!lstack_empty(ls2)) free(lpop(ls2));
    while (!lstack_empty(ls3)) free(lpop(ls3));
    
    lstack_destroy(ls1, test_oc.deinit);
    lstack_destroy(ls2, test_oc.deinit);
    lstack_destroy(ls3, test_oc.deinit);
    printf("✓ All stacks cleaned up successfully\n");
}

/* Test 10: Edge Cases */
static void test_edge_cases(void) {
    printf("\n=== Test 10: Edge Cases ===\n");
    
    struct syspool item_pool = {0};
    struct allocator_concept item_allocator = create_allocator(&item_pool);
    
    struct lstack* ls = lstack_create(&item_allocator);
    assert(ls != NULL);
    
    /* Single item push and pop */
    test_data_t* single = create_test_data(42, "Single");
    lpush(ls, single);
    assert(lstack_size(ls) == 1);
    
    test_data_t* popped = (test_data_t*)lpop(ls);
    assert(popped == single);
    assert(lstack_empty(ls) == 1);
    free(popped);
    printf("✓ Single item push/pop works correctly\n");
    
    /* Multiple pops from empty */
    assert(lpop(ls) == NULL);
    assert(lpop(ls) == NULL);
    assert(lpop(ls) == NULL);
    printf("✓ Multiple pops from empty stack return NULL\n");
    
    /* Push after emptying */
    test_data_t* after = create_test_data(99, "After");
    lpush(ls, after);
    assert(lstack_size(ls) == 1);
    assert(ltop(ls) == after);
    free(lpop(ls));
    printf("✓ Push after emptying works correctly\n");
    
    lstack_destroy(ls, test_oc.deinit);
}

/* Test 11: NULL Allocator Handling */
static void test_null_allocator(void) {
    
}

/* Main test runner */
int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║   LSTACK API TEST SUITE               ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_create_destroy();
    test_push_pop();
    test_top();
    test_empty_stack();
    test_walk();
    test_stress();
    test_destroy_with_items();
    test_alternating_ops();
    test_multiple_stacks();
    test_edge_cases();
    test_null_allocator();
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║   ALL TESTS PASSED SUCCESSFULLY! ✓   ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    return 0;
}