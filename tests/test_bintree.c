#include <ds/trees/bintree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test node structure - intrusive design
struct test_node {
    int value;
    struct bintree hook;
};

// Helper: Create a test node
struct test_node* create_node(int value) {
    struct test_node *node = malloc(sizeof(*node));
    if (node) {
        node->value = value;
        bintree_init(&node->hook, NULL, NULL, NULL);
    }
    return node;
}

// Helper: Free node (for object_concept)
void test_node_deinit(void *node) {
    struct test_node *tn = bintree_get_entry(node, struct test_node, hook);
    free(tn);
}

// Comparison callback for search
int test_cmp_int(const void *key, const struct bintree *node) {
    int search_val = *(const int *)key;
    struct test_node *tn = bintree_get_entry(node, struct test_node, hook);
    return search_val - tn->value;
}

// Test counters
static int test_count = 0;
static int test_passed = 0;

#define TEST_START(name) \
    do { \
        printf("\n=== Testing %s ===\n", name); \
        test_count++; \
    } while(0)

#define ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            printf("  FAIL: %s\n", msg); \
            return false; \
        } \
    } while(0)

#define TEST_END() \
    do { \
        printf("  PASS\n"); \
        test_passed++; \
        return true; \
    } while(0)

// Handler for traversal testing
static int traversal_counter = 0;
static int traversal_values[100];

void count_handler(struct bintree *node, void *context) {
    (void)context;
    struct test_node *tn = bintree_get_entry(node, struct test_node, hook);
    traversal_values[traversal_counter++] = tn->value;
}

// Test 1: Basic initialization
bool test_init(void) {
    TEST_START("bintree_init");
    
    struct bintree tree;
    bintree_init(&tree, NULL, NULL, NULL);
    
    ASSERT(tree.parent == NULL, "Parent should be NULL");
    ASSERT(tree.left == NULL, "Left should be NULL");
    ASSERT(tree.right == NULL, "Right should be NULL");
    
    TEST_END();
}

// Test 2: Getters and setters
bool test_getters_setters(void) {
    TEST_START("getters and setters");
    
    struct test_node *root = create_node(10);
    struct test_node *left = create_node(5);
    struct test_node *right = create_node(15);
    
    bintree_set_left(&root->hook, &left->hook);
    bintree_set_right(&root->hook, &right->hook);
    bintree_set_parent(&left->hook, &root->hook);
    bintree_set_parent(&right->hook, &root->hook);
    
    ASSERT(bintree_get_left(&root->hook) == &left->hook, "Left child getter");
    ASSERT(bintree_get_right(&root->hook) == &right->hook, "Right child getter");
    ASSERT(bintree_get_parent(&left->hook) == &root->hook, "Parent getter (left)");
    ASSERT(bintree_get_parent(&right->hook) == &root->hook, "Parent getter (right)");
    
    free(root);
    free(left);
    free(right);
    
    TEST_END();
}

// Test 3: Root detection and retrieval
bool test_root_functions(void) {
    TEST_START("bintree_is_root and bintree_get_root");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n3 = create_node(3);
    
    // Single node is root
    ASSERT(bintree_is_root(&n10->hook), "Single node should be root");
    ASSERT(bintree_get_root(&n10->hook) == &n10->hook, "Get root of single node");
    
    // Build tree
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    
    ASSERT(bintree_is_root(&n10->hook), "Root should be root");
    ASSERT(!bintree_is_root(&n5->hook), "Child should not be root");
    ASSERT(!bintree_is_root(&n3->hook), "Grandchild should not be root");
    
    ASSERT(bintree_get_root(&n3->hook) == &n10->hook, "Get root from leaf");
    ASSERT(bintree_get_root(&n5->hook) == &n10->hook, "Get root from middle");
    
    free(n10);
    free(n5);
    free(n3);
    
    TEST_END();
}

// Test 4: Detach operation
bool test_detach(void) {
    TEST_START("bintree_detach");
    
    /*
     *        10              10
     *       /  \            /
     *      5    15   =>    5
     *     /                /
     *    3                3
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    
    // Detach n15
    bintree_detach(&n15->hook);
    
    ASSERT(bintree_get_right(&n10->hook) == NULL, "Right child should be NULL");
    ASSERT(bintree_get_parent(&n15->hook) == NULL, "Detached node parent should be NULL");
    ASSERT(bintree_is_root(&n15->hook), "Detached node should be root");
    
    // Verify rest of tree intact
    ASSERT(bintree_get_left(&n10->hook) == &n5->hook, "Left subtree intact");
    ASSERT(bintree_get_left(&n5->hook) == &n3->hook, "Grandchild intact");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    
    TEST_END();
}

// Test 5: Replace operation
bool test_replace(void) {
    TEST_START("bintree_replace");
    
    /*
     *        10              10
     *       /  \            /  \
     *      5    15   =>   20    15
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n20 = create_node(20);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    
    // Replace n5 with n20
    bintree_replace(&n5->hook, &n20->hook);
    
    ASSERT(bintree_get_left(&n10->hook) == &n20->hook, "New node in position");
    ASSERT(bintree_get_parent(&n20->hook) == &n10->hook, "New node parent set");
    ASSERT(bintree_get_parent(&n5->hook) == NULL, "Old node parent cleared");
    
    free(n10);
    free(n5);
    free(n15);
    free(n20);
    
    TEST_END();
}

// Test 6: Replace operation (swap was removed from API)
bool test_replace_advanced(void) {
    TEST_START("bintree_replace (advanced)");
    
    /*
     *        10              10
     *       /  \            /  \
     *      5    15   =>   20    15
     *     /                /
     *    3                3
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n20 = create_node(20);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    
    // Replace n5 with n20, should preserve children
    bintree_replace(&n5->hook, &n20->hook);
    
    ASSERT(bintree_get_left(&n10->hook) == &n20->hook, "New node in position");
    ASSERT(bintree_get_parent(&n20->hook) == &n10->hook, "New node parent set");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n20);
    
    TEST_END();
}

// Test 7: Search operation
bool test_search(void) {
    TEST_START("bintree_search");
    
    /*
     * BST structure:
     *        10
     *       /  \
     *      5    15
     *     / \
     *    3   7
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n7 = create_node(7);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_right(&n5->hook, &n7->hook);
    
    // Search for existing values
    int key = 7;
    struct bintree *result = bintree_search(&n10->hook, &key, test_cmp_int);
    ASSERT(result == &n7->hook, "Found value 7");
    
    key = 15;
    result = bintree_search(&n10->hook, &key, test_cmp_int);
    ASSERT(result == &n15->hook, "Found value 15");
    
    key = 10;
    result = bintree_search(&n10->hook, &key, test_cmp_int);
    ASSERT(result == &n10->hook, "Found root value 10");
    
    // Search for non-existing value
    key = 100;
    result = bintree_search(&n10->hook, &key, test_cmp_int);
    ASSERT(result == NULL, "Not found returns NULL");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n7);
    
    TEST_END();
}

// Test 8: Tree structure building
bool test_tree_structure(void) {
    TEST_START("tree structure");
    
    /*
     * Build tree:
     *        10
     *       /  \
     *      5    15
     *     / \
     *    3   7
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n7 = create_node(7);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_right(&n5->hook, &n7->hook);
    
    ASSERT(bintree_get_left(bintree_get_left(&n10->hook)) == &n3->hook, 
           "Grandchild access");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n7);
    
    TEST_END();
}

// Test 9: Size calculation
bool test_size(void) {
    TEST_START("bintree_size");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    
    // Single node
    ASSERT(bintree_size(&n10->hook) == 1, "Single node size");
    
    // Three nodes
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    
    ASSERT(bintree_size(&n10->hook) == 3, "Three node size");
    
    free(n10);
    free(n5);
    free(n15);
    
    TEST_END();
}

// Test 10: Level calculation
bool test_level(void) {
    TEST_START("bintree_level");
    
    /*
     *        10      (level 0)
     *       /  \
     *      5    15   (level 1)
     *     /
     *    3           (level 2)
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    
    struct bintree *root = NULL;
    
    ASSERT(bintree_level(&n10->hook, &root) == 0, "Root is level 0");
    ASSERT(root == &n10->hook, "Root pointer set correctly");
    
    ASSERT(bintree_level(&n5->hook, NULL) == 1, "Child is level 1");
    ASSERT(bintree_level(&n15->hook, NULL) == 1, "Other child is level 1");
    ASSERT(bintree_level(&n3->hook, NULL) == 2, "Grandchild is level 2");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    
    TEST_END();
}

// Test 11: Height calculation
bool test_height(void) {
    TEST_START("bintree_height");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n3 = create_node(3);
    
    // Single node has height 0
    ASSERT(bintree_height(&n10->hook) == 0, "Single node height");
    
    // NULL tree has height -1
    ASSERT(bintree_height(NULL) == -1, "NULL tree height");
    
    // Add left child - height 1
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    ASSERT(bintree_height(&n10->hook) == 1, "Two level height");
    
    // Add another level - height 2
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    ASSERT(bintree_height(&n10->hook) == 2, "Three level height");
    
    free(n10);
    free(n5);
    free(n3);
    
    TEST_END();
}

// Test 12: Balance factor
bool test_balance_factor(void) {
    TEST_START("bintree_balance_factor");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    
    // Balanced tree
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    
    ASSERT(bintree_balance_factor(&n10->hook) == 0, "Balanced tree");
    
    // Left-heavy tree
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    
    ASSERT(bintree_balance_factor(&n10->hook) == 1, "Left-heavy tree");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    
    TEST_END();
}

// Test 13: Inorder traversal with new iterator API
bool test_inorder_iteration(void) {
    TEST_START("bintree_inorder_prev/next");
    
    /*
     *        10
     *       /  \
     *      5    15
     *     / \
     *    3   7
     * Inorder: 3, 5, 7, 10, 15
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n7 = create_node(7);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_right(&n5->hook, &n7->hook);
    
    // Test forward iteration starting from first
    struct bintree *first = bintree_first_inorder(&n10->hook);
    struct test_node *tn = bintree_get_entry(first, struct test_node, hook);
    ASSERT(tn->value == 3, "First inorder is leftmost");
    
    struct bintree *curr = first;
    int expected[] = {3, 5, 7, 10, 15};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected[i], "Inorder next iteration");
        curr = bintree_inorder_next(curr);
    }
    ASSERT(curr == NULL, "End of iteration");
    
    // Test backward iteration
    curr = &n15->hook;
    int expected_rev[] = {15, 10, 7, 5, 3};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected_rev[i], "Inorder prev iteration");
        curr = bintree_inorder_prev(curr);
    }
    ASSERT(curr == NULL, "Start of iteration");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n7);
    
    TEST_END();
}

// Test 14: Preorder and postorder iterators
bool test_preorder_postorder_iterators(void) {
    TEST_START("preorder and postorder iterators");
    
    /*
     *        10
     *       /  \
     *      5    15
     *     / \
     *    3   7
     * Preorder: 10, 5, 3, 7, 15
     * Postorder: 3, 7, 5, 15, 10
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n7 = create_node(7);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_right(&n5->hook, &n7->hook);
    
    // Test preorder iteration
    struct bintree *first_pre = bintree_first_preorder(&n10->hook);
    struct test_node *tn = bintree_get_entry(first_pre, struct test_node, hook);
    ASSERT(tn->value == 10, "First preorder is root");
    
    struct bintree *curr = first_pre;
    int expected_pre[] = {10, 5, 3, 7, 15};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected_pre[i], "Preorder next iteration");
        curr = bintree_preorder_next(curr);
    }
    ASSERT(curr == NULL, "End of preorder iteration");
    
    // Test postorder iteration
    struct bintree *first_post = bintree_first_postorder(&n10->hook);
    tn = bintree_get_entry(first_post, struct test_node, hook);
    ASSERT(tn->value == 3, "First postorder is leftmost leaf");
    
    curr = first_post;
    int expected_post[] = {3, 7, 5, 15, 10};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected_post[i], "Postorder next iteration");
        curr = bintree_postorder_next(curr);
    }
    ASSERT(curr == NULL, "End of postorder iteration");
    
    // Test preorder backward iteration
    curr = &n15->hook;
    int expected_pre_rev[] = {15, 7, 3, 5, 10};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected_pre_rev[i], "Preorder prev iteration");
        curr = bintree_preorder_prev(curr);
    }
    ASSERT(curr == NULL, "Start of preorder iteration");
    
    // Test postorder backward iteration
    curr = &n10->hook;
    int expected_post_rev[] = {10, 15, 5, 7, 3};
    for (int i = 0; i < 5; i++) {
        tn = bintree_get_entry(curr, struct test_node, hook);
        ASSERT(tn->value == expected_post_rev[i], "Postorder prev iteration");
        curr = bintree_postorder_prev(curr);
    }
    ASSERT(curr == NULL, "Start of postorder iteration");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n7);
    
    TEST_END();
}

// Test 15: Traversal orders
bool test_traversal_orders(void) {
    TEST_START("bintree_traverse");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    
    // Inorder: 5, 10, 15
    traversal_counter = 0;
    bintree_traverse(&n10->hook, NULL, count_handler, INORDER);
    ASSERT(traversal_counter == 3, "Inorder count");
    ASSERT(traversal_values[0] == 5 && traversal_values[1] == 10 && 
           traversal_values[2] == 15, "Inorder sequence");
    
    // Preorder: 10, 5, 15
    traversal_counter = 0;
    bintree_traverse(&n10->hook, NULL, count_handler, PREORDER);
    ASSERT(traversal_values[0] == 10 && traversal_values[1] == 5 && 
           traversal_values[2] == 15, "Preorder sequence");
    
    // Postorder: 5, 15, 10
    traversal_counter = 0;
    bintree_traverse(&n10->hook, NULL, count_handler, POSTORDER);
    ASSERT(traversal_values[0] == 5 && traversal_values[1] == 15 && 
           traversal_values[2] == 10, "Postorder sequence");
    
    free(n10);
    free(n5);
    free(n15);
    
    TEST_END();
}

// Test 16: BFS traversal
bool test_bfs(void) {
    TEST_START("bintree_bfs");
    
    /*
     *        10
     *       /  \
     *      5    15
     *     / \
     *    3   7
     * BFS: 10, 5, 15, 3, 7
     */
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    struct test_node *n3 = create_node(3);
    struct test_node *n7 = create_node(7);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    bintree_set_left(&n5->hook, &n3->hook);
    bintree_set_right(&n5->hook, &n7->hook);
    bintree_set_parent(&n3->hook, &n5->hook);
    bintree_set_parent(&n7->hook, &n5->hook);
    
    traversal_counter = 0;
    bintree_bfs(&n10->hook, NULL, count_handler);
    
    ASSERT(traversal_counter == 5, "BFS visited all nodes");
    ASSERT(traversal_values[0] == 10, "BFS level 0");
    ASSERT((traversal_values[1] == 5 && traversal_values[2] == 15) ||
           (traversal_values[1] == 15 && traversal_values[2] == 5), "BFS level 1");
    ASSERT((traversal_values[3] == 3 && traversal_values[4] == 7) ||
           (traversal_values[3] == 7 && traversal_values[4] == 3), "BFS level 2");
    
    free(n10);
    free(n5);
    free(n15);
    free(n3);
    free(n7);
    
    TEST_END();
}

// Test 17: DFS traversal
bool test_dfs(void) {
    TEST_START("bintree_dfs");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    
    traversal_counter = 0;
    bintree_dfs(&n10->hook, NULL, count_handler);
    
    ASSERT(traversal_counter == 3, "DFS visited all nodes");
    // DFS should visit in preorder-like manner
    ASSERT(traversal_values[0] == 10, "DFS starts at root");
    
    free(n10);
    free(n5);
    free(n15);
    
    TEST_END();
}

// Test 18: Clear with object_concept
bool test_clear(void) {
    TEST_START("bintree_deinit");
    
    struct test_node *n10 = create_node(10);
    struct test_node *n5 = create_node(5);
    struct test_node *n15 = create_node(15);
    
    bintree_set_left(&n10->hook, &n5->hook);
    bintree_set_right(&n10->hook, &n15->hook);
    bintree_set_parent(&n5->hook, &n10->hook);
    bintree_set_parent(&n15->hook, &n10->hook);
    
    struct object_concept oc = {
        .deinit = test_node_deinit,
        .init = NULL
    };
    
    bintree_deinit(&n10->hook, &oc);
    
    printf("  Tree cleared successfully\n");
    TEST_END();
}

// Main test runner
int main(void) {
    printf("Binary Tree Test Suite\n");
    printf("======================\n");
    
    test_init();
    test_getters_setters();
    test_root_functions();
    test_detach();
    test_replace();
    test_replace_advanced();
    test_search();
    test_tree_structure();
    test_size();
    test_level();
    test_height();
    test_balance_factor();
    test_inorder_iteration();
    test_preorder_postorder_iterators();
    test_traversal_orders();
    test_bfs();
    test_dfs();
    test_clear();
    
    printf("\n======================\n");
    printf("Results: %d/%d tests passed\n", test_passed, test_count);
    
    return (test_passed == test_count) ? 0 : 1;
}