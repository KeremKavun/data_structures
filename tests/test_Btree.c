/**
 * @file btree_test.c
 * @brief Comprehensive test suite for B-tree implementation
 * 
 * Tests cover:
 * - Edge cases (empty tree, single element, duplicates)
 * - Boundary conditions (min/max order, capacity limits)
 * - Large datasets (10k+ elements)
 * - Split and merge operations
 * - Memory management
 */

#include <ds/trees/Btree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

/* ========================================================================
 * Test Infrastructure
 * ======================================================================== */

#define TEST_PASSED "\033[32m[PASS]\033[0m"
#define TEST_FAILED "\033[31m[FAIL]\033[0m"
#define TEST_SECTION "\033[36m[====]\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_TEST(condition, message) do { \
    if (condition) { \
        printf("%s %s\n", TEST_PASSED, message); \
        tests_passed++; \
    } else { \
        printf("%s %s\n", TEST_FAILED, message); \
        tests_failed++; \
    } \
} while(0)

/* ========================================================================
 * Allocator Setup using syspool
 * ======================================================================== */

static struct allocator_concept create_btree_allocator(size_t order) {
    struct syspool *pool = (struct syspool*)malloc(sizeof(struct syspool));
    pool->obj_size = Btree_node_sizeof(order);
    
    struct allocator_concept ac;
    ac.allocator = pool;
    ac.alloc = sysalloc;
    ac.free = sysfree;
    return ac;
}

static void destroy_allocator(struct allocator_concept *ac) {
    if (ac->allocator) {
        free(ac->allocator);
        ac->allocator = NULL;
    }
}

/* ========================================================================
 * Object Concept for Data Management
 * ======================================================================== */

static int int_init(void *object, void *args) {
    // For integer test data, no special initialization needed
    // Just copy the value if args provided
    if (object && args) {
        *(int*)object = *(int*)args;
        return 0;
    }
    return 0;
}

static void int_deinit(void *obj) {
    if (obj) {
        free(obj);
    }
}

static struct object_concept create_int_object_concept(void) {
    struct object_concept oc;
    oc.init = int_init;
    oc.deinit = int_deinit;
    return oc;
}

/* ========================================================================
 * Comparison Functions
 * ======================================================================== */

static int int_compare(const void *key, const void *data) {
    int k = *(const int*)key;
    int d = *(const int*)data;
    return (k > d) - (k < d);
}

static int reverse_int_compare(const void *key, const void *data) {
    int k = *(const int*)key;
    int d = *(const int*)data;
    return (d > k) - (d < k);
}

/* ========================================================================
 * Test Data Structures
 * ======================================================================== */

typedef struct {
    int *values;
    size_t count;
} TraversalContext;

static void collect_handler(void *data, void *context) {
    TraversalContext *ctx = (TraversalContext*)context;
    int val = *(int*)data;
    ctx->values[ctx->count++] = val;
}

static void print_handler(void *data, void *context) {
    (void)context;
    printf("%d ", *(int*)data);
}

/* ========================================================================
 * Helper Functions
 * ======================================================================== */

static int* create_int(int value) {
    int *p = (int*)malloc(sizeof(int));
    *p = value;
    return p;
}

static int is_sorted_ascending(int *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (arr[i] < arr[i-1]) return 0;
    }
    return 1;
}

static int is_sorted_descending(int *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (arr[i] > arr[i-1]) return 0;
    }
    return 1;
}

/* ========================================================================
 * Test Cases
 * ======================================================================== */

void test_create_destroy(void) {
    printf("\n%s Test: Create and Destroy\n", TEST_SECTION);
    
    struct object_concept oc = create_int_object_concept();
    
    // Test creation with various orders
    struct allocator_concept ac3 = create_btree_allocator(3);
    struct Btree *tree3 = Btree_create(3, int_compare, &ac3);
    ASSERT_TEST(tree3 != NULL, "Create B-tree of order 3");
    ASSERT_TEST(Btree_empty(tree3), "New tree is empty");
    ASSERT_TEST(Btree_size(tree3) == 0, "New tree has size 0");
    Btree_destroy(tree3, &oc);
    destroy_allocator(&ac3);
    
    struct allocator_concept ac5 = create_btree_allocator(5);
    struct Btree *tree5 = Btree_create(5, int_compare, &ac5);
    ASSERT_TEST(tree5 != NULL, "Create B-tree of order 5");
    Btree_destroy(tree5, &oc);
    destroy_allocator(&ac5);
    
    struct allocator_concept ac100 = create_btree_allocator(100);
    struct Btree *tree100 = Btree_create(100, int_compare, &ac100);
    ASSERT_TEST(tree100 != NULL, "Create B-tree of order 100");
    Btree_destroy(tree100, &oc);
    destroy_allocator(&ac100);
}

void test_single_element(void) {
    printf("\n%s Test: Single Element Operations\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(3);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(3, int_compare, &ac);
    
    int *val = create_int(42);
    enum trees_status status = Btree_add(tree, val);
    
    ASSERT_TEST(status == TREES_OK, "Add single element");
    ASSERT_TEST(!Btree_empty(tree), "Tree not empty after add");
    ASSERT_TEST(Btree_size(tree) == 1, "Size is 1 after add");
    
    int search_key = 42;
    void *found = Btree_search(tree, &search_key);
    ASSERT_TEST(found != NULL && *(int*)found == 42, "Search finds element");
    
    int not_found_key = 99;
    found = Btree_search(tree, &not_found_key);
    ASSERT_TEST(found == NULL, "Search returns NULL for missing element");
    
    int remove_key = 42;
    void *removed = Btree_remove(tree, &remove_key);
    ASSERT_TEST(removed != NULL && *(int*)removed == 42, "Remove returns element");
    ASSERT_TEST(Btree_empty(tree), "Tree empty after remove");
    ASSERT_TEST(Btree_size(tree) == 0, "Size is 0 after remove");
    
    free(removed);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_sequential_insertion(void) {
    printf("\n%s Test: Sequential Insertion (Ascending)\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    const int N = 100;
    for (int i = 0; i < N; i++) {
        int *val = create_int(i);
        enum trees_status status = Btree_add(tree, val);
        if (status != TREES_OK) {
            printf("Failed to add %d, status: %d\n", i, status);
        }
    }
    
    ASSERT_TEST(Btree_size(tree) == N, "All sequential elements added");
    
    // Verify all elements are searchable
    int all_found = 1;
    for (int i = 0; i < N; i++) {
        void *found = Btree_search(tree, &i);
        if (found == NULL || *(int*)found != i) {
            all_found = 0;
            break;
        }
    }
    ASSERT_TEST(all_found, "All sequential elements are searchable");
    
    // Test traversal order
    TraversalContext ctx;
    ctx.values = (int*)malloc(N * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    
    ASSERT_TEST(ctx.count == N, "Traversal visits all elements");
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Traversal is in order");
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_reverse_insertion(void) {
    printf("\n%s Test: Sequential Insertion (Descending)\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    const int N = 100;
    for (int i = N - 1; i >= 0; i--) {
        int *val = create_int(i);
        Btree_add(tree, val);
    }
    
    ASSERT_TEST(Btree_size(tree) == N, "All reverse elements added");
    
    // Test traversal order
    TraversalContext ctx;
    ctx.values = (int*)malloc(N * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Traversal is still in order");
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_random_insertion(void) {
    printf("\n%s Test: Random Insertion\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(7);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(7, int_compare, &ac);
    
    const int N = 500;
    int *expected = (int*)malloc(N * sizeof(int));
    
    srand(12345); // Fixed seed for reproducibility
    for (int i = 0; i < N; i++) {
        expected[i] = rand() % 10000;
        int *val = create_int(expected[i]);
        Btree_add(tree, val);
    }
    
    ASSERT_TEST(Btree_size(tree) == N, "All random elements added");
    
    // Test traversal produces sorted output
    TraversalContext ctx;
    ctx.values = (int*)malloc(N * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Random insertion still sorted");
    
    free(ctx.values);
    free(expected);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_duplicate_handling(void) {
    printf("\n%s Test: Duplicate Handling\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    // Add duplicate values
    for (int i = 0; i < 5; i++) {
        int *val = create_int(42);
        enum trees_status status = Btree_add(tree, val);
        if (i == 0) {
            ASSERT_TEST(status == TREES_OK, "First duplicate added");
        } else {
            // Depending on implementation, this might be TREES_DUPLICATE
            // or it might replace - test both scenarios
            int is_valid = (status == TREES_OK || status == TREES_DUPLICATE_KEY);
            ASSERT_TEST(is_valid, "Duplicate handling consistent");
            if (status == TREES_DUPLICATE_KEY) {
                free(val); // Free if not added
            }
        }
    }
    
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_removal_patterns(void) {
    printf("\n%s Test: Removal Patterns\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    // Insert elements
    const int N = 50;
    for (int i = 0; i < N; i++) {
        int *val = create_int(i);
        Btree_add(tree, val);
    }
    
    // Remove every other element
    int removed_count = 0;
    for (int i = 0; i < N; i += 2) {
        void *removed = Btree_remove(tree, &i);
        if (removed != NULL) {
            free(removed);
            removed_count++;
        }
    }
    
    ASSERT_TEST(removed_count == N/2, "Removed half elements");
    ASSERT_TEST(Btree_size(tree) == N - removed_count, "Size correct after removals");
    
    // Verify remaining elements
    int all_correct = 1;
    for (int i = 1; i < N; i += 2) {
        void *found = Btree_search(tree, &i);
        if (found == NULL) {
            all_correct = 0;
            break;
        }
    }
    ASSERT_TEST(all_correct, "Remaining elements still searchable");
    
    // Verify removed elements are gone
    int all_gone = 1;
    for (int i = 0; i < N; i += 2) {
        void *found = Btree_search(tree, &i);
        if (found != NULL) {
            all_gone = 0;
            break;
        }
    }
    ASSERT_TEST(all_gone, "Removed elements not found");
    
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_large_dataset(void) {
    printf("\n%s Test: Large Dataset (10,000 elements)\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(20);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(20, int_compare, &ac);
    
    const int N = 10000;
    printf("Inserting %d elements...\n", N);
    
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        int *val = create_int(i);
        Btree_add(tree, val);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    ASSERT_TEST(Btree_size(tree) == N, "Large dataset: all elements added");
    printf("Insertion time: %.3f seconds\n", insert_time);
    
    // Search test
    start = clock();
    int search_success = 1;
    for (int i = 0; i < N; i += 100) {
        void *found = Btree_search(tree, &i);
        if (found == NULL) {
            search_success = 0;
            break;
        }
    }
    end = clock();
    double search_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    ASSERT_TEST(search_success, "Large dataset: sampling search successful");
    printf("Search time (100 samples): %.3f seconds\n", search_time);
    
    // Traversal test
    start = clock();
    TraversalContext ctx;
    ctx.values = (int*)malloc(N * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    end = clock();
    double traversal_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    ASSERT_TEST(ctx.count == N, "Large dataset: traversal visits all");
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Large dataset: traversal sorted");
    printf("Traversal time: %.3f seconds\n", traversal_time);
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_stress_add_remove(void) {
    printf("\n%s Test: Stress Test (Mixed Add/Remove)\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(10);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(10, int_compare, &ac);
    
    const int OPERATIONS = 5000;
    srand(54321);
    
    int current_max = 0;
    for (int i = 0; i < OPERATIONS; i++) {
        if (rand() % 2 == 0 || Btree_empty(tree)) {
            // Add
            int *val = create_int(current_max++);
            Btree_add(tree, val);
        } else {
            // Remove random element
            int remove_key = rand() % current_max;
            void *removed = Btree_remove(tree, &remove_key);
            if (removed) free(removed);
        }
    }
    
    ASSERT_TEST(Btree_size(tree) > 0, "Stress test: tree not empty");
    
    // Verify integrity via traversal
    TraversalContext ctx;
    size_t size = Btree_size(tree);
    ctx.values = (int*)malloc(size * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    
    ASSERT_TEST(ctx.count == size, "Stress test: traversal count matches size");
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Stress test: still sorted");
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_different_orders(void) {
    printf("\n%s Test: Different B-tree Orders\n", TEST_SECTION);
    
    struct object_concept oc = create_int_object_concept();
    
    size_t orders[] = {3, 5, 7, 11, 17, 31, 64};
    size_t num_orders = sizeof(orders) / sizeof(orders[0]);
    
    const int N = 200;
    
    for (size_t i = 0; i < num_orders; i++) {
        struct allocator_concept ac = create_btree_allocator(orders[i]);
        struct Btree *tree = Btree_create(orders[i], int_compare, &ac);
        
        for (int j = 0; j < N; j++) {
            int *val = create_int(j);
            Btree_add(tree, val);
        }
        
        int ok = (Btree_size(tree) == N);
        
        if (ok) {
            TraversalContext ctx;
            ctx.values = (int*)malloc(N * sizeof(int));
            ctx.count = 0;
            Btree_walk(tree, &ctx, collect_handler);
            ok = is_sorted_ascending(ctx.values, ctx.count);
            free(ctx.values);
        }
        
        char msg[100];
        snprintf(msg, sizeof(msg), "Order %zu: correct behavior", orders[i]);
        ASSERT_TEST(ok, msg);
        
        Btree_destroy(tree, &oc);
        destroy_allocator(&ac);
    }
}

void test_boundary_values(void) {
    printf("\n%s Test: Boundary Values\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    // Test with extreme values
    int *min_val = create_int(INT_MIN);
    int *max_val = create_int(INT_MAX);
    int *zero_val = create_int(0);
    
    Btree_add(tree, min_val);
    Btree_add(tree, max_val);
    Btree_add(tree, zero_val);
    
    ASSERT_TEST(Btree_size(tree) == 3, "Boundary values added");
    
    int search_min = INT_MIN;
    void *found_min = Btree_search(tree, &search_min);
    ASSERT_TEST(found_min != NULL && *(int*)found_min == INT_MIN, "Found INT_MIN");
    
    int search_max = INT_MAX;
    void *found_max = Btree_search(tree, &search_max);
    ASSERT_TEST(found_max != NULL && *(int*)found_max == INT_MAX, "Found INT_MAX");
    
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_empty_operations(void) {
    printf("\n%s Test: Operations on Empty Tree\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    int key = 42;
    void *result = Btree_search(tree, &key);
    ASSERT_TEST(result == NULL, "Search on empty tree returns NULL");
    
    result = Btree_remove(tree, &key);
    ASSERT_TEST(result == NULL, "Remove on empty tree returns NULL");
    
    TraversalContext ctx;
    ctx.values = (int*)malloc(10 * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    ASSERT_TEST(ctx.count == 0, "Walk on empty tree visits nothing");
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_node_sizeof(void) {
    printf("\n%s Test: Node Size Calculation\n", TEST_SECTION);
    
    // Test that node size calculation is consistent
    size_t size3 = Btree_node_sizeof(3);
    size_t size5 = Btree_node_sizeof(5);
    size_t size100 = Btree_node_sizeof(100);
    
    ASSERT_TEST(size3 > 0, "Order 3 node size > 0");
    ASSERT_TEST(size5 > size3, "Order 5 node size > Order 3");
    ASSERT_TEST(size100 > size5, "Order 100 node size > Order 5");
    
    printf("Node sizes: order=3: %zu, order=5: %zu, order=100: %zu\n", 
           size3, size5, size100);
}

void test_extreme_removal(void) {
    printf("\n%s Test: Extreme Removal (Remove All)\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(7);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(7, int_compare, &ac);
    
    const int N = 100;
    
    // Insert elements
    for (int i = 0; i < N; i++) {
        int *val = create_int(i);
        Btree_add(tree, val);
    }
    
    ASSERT_TEST(Btree_size(tree) == N, "All elements inserted");
    
    // Remove all elements in random order
    int *removal_order = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        removal_order[i] = i;
    }
    
    // Shuffle
    srand(99999);
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = removal_order[i];
        removal_order[i] = removal_order[j];
        removal_order[j] = temp;
    }
    
    // Remove all
    int all_removed = 1;
    for (int i = 0; i < N; i++) {
        void *removed = Btree_remove(tree, &removal_order[i]);
        if (removed == NULL) {
            all_removed = 0;
            printf("Failed to remove %d\n", removal_order[i]);
            break;
        }
        free(removed);
    }
    
    ASSERT_TEST(all_removed, "All elements removed successfully");
    ASSERT_TEST(Btree_empty(tree), "Tree empty after removing all");
    ASSERT_TEST(Btree_size(tree) == 0, "Size is 0 after removing all");
    
    free(removal_order);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

void test_alternating_ops(void) {
    printf("\n%s Test: Alternating Add/Remove Operations\n", TEST_SECTION);
    
    struct allocator_concept ac = create_btree_allocator(5);
    struct object_concept oc = create_int_object_concept();
    struct Btree *tree = Btree_create(5, int_compare, &ac);
    
    // Add, remove, add, remove pattern
    const int CYCLES = 50;
    int integrity_ok = 1;
    
    for (int cycle = 0; cycle < CYCLES; cycle++) {
        // Add 10 elements
        for (int i = 0; i < 10; i++) {
            int *val = create_int(cycle * 10 + i);
            Btree_add(tree, val);
        }
        
        // Remove 5 elements
        for (int i = 0; i < 5; i++) {
            int key = cycle * 10 + i;
            void *removed = Btree_remove(tree, &key);
            if (removed) {
                free(removed);
            } else {
                integrity_ok = 0;
            }
        }
    }
    
    ASSERT_TEST(integrity_ok, "Alternating operations maintain integrity");
    ASSERT_TEST(Btree_size(tree) == CYCLES * 5, "Correct size after alternating ops");
    
    // Verify remaining elements
    TraversalContext ctx;
    ctx.values = (int*)malloc(Btree_size(tree) * sizeof(int));
    ctx.count = 0;
    Btree_walk(tree, &ctx, collect_handler);
    
    ASSERT_TEST(is_sorted_ascending(ctx.values, ctx.count), "Still sorted after alternating ops");
    
    free(ctx.values);
    Btree_destroy(tree, &oc);
    destroy_allocator(&ac);
}

/* ========================================================================
 * Main Test Runner
 * ======================================================================== */

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║         B-TREE COMPREHENSIVE TEST SUITE                ║\n");
    printf("║              (Using syspool allocator)                 ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    test_create_destroy();
    test_node_sizeof();
    test_empty_operations();
    test_single_element();
    test_sequential_insertion();
    test_reverse_insertion();
    test_random_insertion();
    test_duplicate_handling();
    test_removal_patterns();
    test_extreme_removal();
    test_boundary_values();
    test_alternating_ops();
    test_different_orders();
    test_large_dataset();
    test_stress_add_remove();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                        ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                     ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d                                     ║\n", tests_passed);
    printf("║  Failed:       %3d                                     ║\n", tests_failed);
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}