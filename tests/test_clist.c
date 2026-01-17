#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* Include the header */
#include <ds/linkedlists/clist.h>
#include <ds/utils/macros.h>

/* Test data structure */
struct test_node {
    int id;
    char name[32];
    struct clist_item hook;
};

int lambda(struct test_node* n)
{
    return n->id == 30;
}

/* Helper functions */
static struct test_node* create_node(int id, const char* name) {
    struct test_node* node = malloc(sizeof(struct test_node));
    if (!node) return NULL;
    
    node->id = id;
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    clist_item_init(&node->hook);
    
    return node;
}

static void free_node(struct test_node* node) {
    free(node);
}

static int condition_id_equals(struct test_node* node, int target_id) {
    return node->id == target_id;
}

#define CONDITION_ID(target) \
    ((int(*)(struct test_node*))condition_id_equals)

/* Test counter */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Test: %s ===\n", name)

#define ASSERT_TRUE(expr, msg) \
    do { \
        if (expr) { \
            printf("✓ %s\n", msg); \
            tests_passed++; \
        } else { \
            printf("✗ %s (line %d)\n", msg, __LINE__); \
            tests_failed++; \
        } \
    } while(0)

#define ASSERT_EQ(a, b, msg) \
    ASSERT_TRUE((a) == (b), msg)

#define ASSERT_NOT_NULL(ptr, msg) \
    ASSERT_TRUE((ptr) != NULL, msg)

#define ASSERT_NULL(ptr, msg) \
    ASSERT_TRUE((ptr) == NULL, msg)

/* Test functions */
void test_init_and_empty() {
    TEST("Initialization and Empty Check");
    
    struct clist cl;
    clist_init(&cl);
    
    ASSERT_EQ(clist_empty(&cl), 1, "New list should be empty");
    ASSERT_EQ(clist_size(&cl), 0, "New list should have size 0");
    ASSERT_NOT_NULL(get_clist_sentinel(&cl), "Sentinel should not be NULL");
}

void test_push_pop_front() {
    TEST("Push and Pop Front Operations");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* n1 = create_node(1, "first");
    struct test_node* n2 = create_node(2, "second");
    struct test_node* n3 = create_node(3, "third");
    
    /* Push front */
    clist_push_front(&cl, &n1->hook);
    ASSERT_EQ(clist_size(&cl), 1, "Size should be 1 after first push");
    ASSERT_EQ(clist_empty(&cl), 0, "List should not be empty");
    
    clist_push_front(&cl, &n2->hook);
    ASSERT_EQ(clist_size(&cl), 2, "Size should be 2 after second push");
    
    clist_push_front(&cl, &n3->hook);
    ASSERT_EQ(clist_size(&cl), 3, "Size should be 3 after third push");
    
    /* Pop front - should get n3, n2, n1 (LIFO) */
    struct clist_item* item = clist_pop_front(&cl);
    struct test_node* node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 3, "First pop should return node 3");
    ASSERT_EQ(clist_size(&cl), 2, "Size should be 2 after first pop");
    free_node(node);
    
    item = clist_pop_front(&cl);
    node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 2, "Second pop should return node 2");
    free_node(node);
    
    item = clist_pop_front(&cl);
    node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 1, "Third pop should return node 1");
    free_node(node);
    
    ASSERT_EQ(clist_empty(&cl), 1, "List should be empty after all pops");
    ASSERT_NULL(clist_pop_front(&cl), "Pop from empty list should return NULL");
}

void test_push_pop_back() {
    TEST("Push and Pop Back Operations");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* n1 = create_node(1, "first");
    struct test_node* n2 = create_node(2, "second");
    struct test_node* n3 = create_node(3, "third");
    
    /* Push back */
    clist_push_back(&cl, &n1->hook);
    clist_push_back(&cl, &n2->hook);
    clist_push_back(&cl, &n3->hook);
    
    ASSERT_EQ(clist_size(&cl), 3, "Size should be 3 after pushes");
    
    /* Pop back - should get n3, n2, n1 (LIFO for back) */
    struct clist_item* item = clist_pop_back(&cl);
    struct test_node* node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 3, "First pop_back should return node 3");
    free_node(node);
    
    item = clist_pop_back(&cl);
    node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 2, "Second pop_back should return node 2");
    free_node(node);
    
    item = clist_pop_back(&cl);
    node = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(node->id, 1, "Third pop_back should return node 1");
    free_node(node);
    
    ASSERT_EQ(clist_empty(&cl), 1, "List should be empty");
}

void test_insert_remove() {
    TEST("Insert Before/After and Remove");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* n1 = create_node(1, "one");
    struct test_node* n2 = create_node(2, "two");
    struct test_node* n3 = create_node(3, "three");
    struct test_node* n4 = create_node(4, "four");
    
    /* Build list: 1 -> 2 -> 3 */
    clist_push_back(&cl, &n1->hook);
    clist_push_back(&cl, &n2->hook);
    clist_push_back(&cl, &n3->hook);
    
    /* Insert 4 before 2: 1 -> 4 -> 2 -> 3 */
    clist_insert_before(&cl, &n2->hook, &n4->hook);
    ASSERT_EQ(clist_size(&cl), 4, "Size should be 4 after insert");
    
    /* Verify order by iterating */
    struct clist_item* pos;
    int expected_ids[] = {1, 4, 2, 3};
    int idx = 0;
    clist_foreach_cw(pos, &cl) {
        struct test_node* node = clist_entry(pos, struct test_node, hook);
        ASSERT_EQ(node->id, expected_ids[idx], "Insert order verification");
        idx++;
    }
    
    /* Remove node 2 */
    clist_remove(&cl, &n2->hook);
    ASSERT_EQ(clist_size(&cl), 3, "Size should be 3 after remove");
    free_node(n2);
    
    /* Remove node 1 */
    clist_remove(&cl, &n1->hook);
    ASSERT_EQ(clist_size(&cl), 2, "Size should be 2 after second remove");
    free_node(n1);
    
    /* Clean up */
    clist_pop_front(&cl);
    free_node(n4);
    clist_pop_front(&cl);
    free_node(n3);
}

void test_iteration() {
    TEST("Iteration Clockwise and Counter-Clockwise");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i] = create_node(i + 1, "node");
        clist_push_back(&cl, &nodes[i]->hook);
    }
    
    /* Test clockwise iteration */
    int count_cw = 0;
    int expected_id = 1;
    struct clist_item* pos;
    clist_foreach_cw(pos, &cl) {
        struct test_node* node = clist_entry(pos, struct test_node, hook);
        ASSERT_EQ(node->id, expected_id, "CW iteration order");
        expected_id++;
        count_cw++;
    }
    ASSERT_EQ(count_cw, 5, "CW iteration should visit all 5 nodes");
    
    /* Test counter-clockwise iteration */
    int count_ccw = 0;
    expected_id = 5;
    clist_foreach_ccw(pos, &cl) {
        struct test_node* node = clist_entry(pos, struct test_node, hook);
        ASSERT_EQ(node->id, expected_id, "CCW iteration order");
        expected_id--;
        count_ccw++;
    }
    ASSERT_EQ(count_ccw, 5, "CCW iteration should visit all 5 nodes");
    
    /* Test foreach_entry macro */
    struct test_node* obj;
    int count_entry = 0;
    expected_id = 1;
    clist_foreach_entry(obj, &cl, hook) {
        ASSERT_EQ(obj->id, expected_id, "foreach_entry iteration order");
        expected_id++;
        count_entry++;
    }
    ASSERT_EQ(count_entry, 5, "foreach_entry should visit all 5 nodes");
    
    /* Clean up */
    for (int i = 0; i < 5; i++) {
        clist_pop_front(&cl);
        free_node(nodes[i]);
    }
}

void test_search() {
    TEST("Search Operations");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i] = create_node((i + 1) * 10, "node");
        clist_push_back(&cl, &nodes[i]->hook);
    }
    
    /* Search forward for existing item using macro */
    struct test_node* result = NULL;
    clist_find_entry_fr(result, &cl, hook, lambda);
    ASSERT_NOT_NULL(result, "Should find node with id=30 using forward search");
    if (result) {
        ASSERT_EQ(result->id, 30, "Found node should have id=30");
    }
    
    /* Search backward for existing item */
    result = NULL;
    clist_find_entry_bk(result, &cl, hook, lambda);
    ASSERT_NOT_NULL(result, "Should find node with id=30 using backward search");
    if (result) {
        ASSERT_EQ(result->id, 30, "Found node should have id=30");
    }
    
    /* Search for non-existing item */
    result = NULL;
    struct clist_item* pos;
    clist_foreach_cw(pos, &cl) {
        struct test_node* node = clist_entry(pos, struct test_node, hook);
        if (node->id == 99) {
            result = node;
            break;
        }
    }
    ASSERT_NULL(result, "Should not find node with id=99");
    
    /* Clean up */
    for (int i = 0; i < 5; i++) {
        clist_pop_front(&cl);
        free_node(nodes[i]);
    }
}

void test_sentinel_operations() {
    TEST("Sentinel Operations");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* n1 = create_node(1, "one");
    struct test_node* n2 = create_node(2, "two");
    struct test_node* n3 = create_node(3, "three");
    
    clist_push_back(&cl, &n1->hook);
    clist_push_back(&cl, &n2->hook);
    clist_push_back(&cl, &n3->hook);
    
    /* Get sentinel */
    struct clist_item* sentinel = get_clist_sentinel(&cl);
    ASSERT_NOT_NULL(sentinel, "Sentinel should not be NULL");
    
    /* Verify sentinel.next points to first node */
    struct clist_item* first = clist_item_next(sentinel);
    struct test_node* first_node = clist_entry(first, struct test_node, hook);
    ASSERT_EQ(first_node->id, 1, "Sentinel.next should point to first node");
    
    /* Verify sentinel.prev points to last node */
    struct clist_item* last = clist_item_prev(sentinel);
    struct test_node* last_node = clist_entry(last, struct test_node, hook);
    ASSERT_EQ(last_node->id, 3, "Sentinel.prev should point to last node");
    
    /* Test item navigation */
    struct clist_item* second = clist_item_next(first);
    struct test_node* second_node = clist_entry(second, struct test_node, hook);
    ASSERT_EQ(second_node->id, 2, "Navigation to second node");
    
    struct clist_item* back_to_first = clist_item_prev(second);
    struct test_node* back_node = clist_entry(back_to_first, struct test_node, hook);
    ASSERT_EQ(back_node->id, 1, "Backward navigation to first node");
    
    /* Clean up */
    for (int i = 0; i < 3; i++) {
        clist_pop_front(&cl);
    }
    free_node(n1);
    free_node(n2);
    free_node(n3);
}

void test_insert_after() {
    TEST("Insert After Operations");
    
    struct clist cl;
    clist_init(&cl);
    
    struct test_node* n1 = create_node(1, "one");
    struct test_node* n2 = create_node(2, "two");
    struct test_node* n3 = create_node(3, "three");
    
    clist_push_back(&cl, &n1->hook);
    clist_push_back(&cl, &n3->hook);
    
    /* Insert n2 after n1: 1 -> 2 -> 3 */
    clist_insert_after(&cl, &n1->hook, &n2->hook);
    ASSERT_EQ(clist_size(&cl), 3, "Size should be 3 after insert_after");
    
    /* Verify order */
    struct clist_item* pos;
    int expected_ids[] = {1, 2, 3};
    int idx = 0;
    clist_foreach_cw(pos, &cl) {
        struct test_node* node = clist_entry(pos, struct test_node, hook);
        ASSERT_EQ(node->id, expected_ids[idx], "Insert after order verification");
        idx++;
    }
    
    /* Clean up */
    for (int i = 0; i < 3; i++) {
        clist_pop_front(&cl);
    }
    free_node(n1);
    free_node(n2);
    free_node(n3);
}

void test_mixed_operations() {
    TEST("Mixed Operations - Real World Scenario");
    
    struct clist cl;
    clist_init(&cl);
    
    /* Build a task queue */
    struct test_node* tasks[3];
    tasks[0] = create_node(100, "high_priority");
    tasks[1] = create_node(50, "medium_priority");
    tasks[2] = create_node(10, "low_priority");
    
    clist_push_back(&cl, &tasks[1]->hook);
    clist_push_back(&cl, &tasks[2]->hook);
    clist_push_front(&cl, &tasks[0]->hook);
    
    ASSERT_EQ(clist_size(&cl), 3, "Queue should have 3 tasks");
    
    /* Process high priority (front) */
    struct clist_item* item = clist_pop_front(&cl);
    struct test_node* task = clist_entry(item, struct test_node, hook);
    ASSERT_EQ(task->id, 100, "Should process high priority first");
    free_node(task);
    
    /* Add urgent task */
    struct test_node* urgent = create_node(200, "urgent");
    clist_push_front(&cl, &urgent->hook);
    
    /* Process all remaining */
    int count = 0;
    while (!clist_empty(&cl)) {
        clist_pop_front(&cl);
        count++;
    }
    ASSERT_EQ(count, 3, "Should process 3 remaining tasks");
    
    free_node(urgent);
    free_node(tasks[1]);
    free_node(tasks[2]);
}

/* Main test runner */
int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║   Circular List Test Suite            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_init_and_empty();
    test_push_pop_front();
    test_push_pop_back();
    test_insert_remove();
    test_insert_after();
    test_iteration();
    test_search();
    test_sentinel_operations();
    test_mixed_operations();
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║          Test Results                  ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║  Passed: %-4d                         ║\n", tests_passed);
    printf("║  Failed: %-4d                         ║\n", tests_failed);
    printf("║  Total:  %-4d                         ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? 1 : 0;
}