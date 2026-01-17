#include <ds/trees/avl.h>
#include <ds/utils/object_concept.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stddef.h>

#define STRESS_COUNT 50000

//───────────────────────────────────────────────
// Data structure with intrusive AVL node
//───────────────────────────────────────────────

typedef struct {
    struct avl_node node;  // Intrusive AVL node MUST be first
    int value;
} IntNode;

// Compile-time check that node is at offset 0
_Static_assert(offsetof(IntNode, node) == 0, "node must be first member of IntNode");

//───────────────────────────────────────────────
// Comparison functions
//───────────────────────────────────────────────

// For AVL operations (node to node comparison)
static int node_cmp(const struct bintree* a, const struct bintree* b)
{
    const IntNode* na = (const IntNode*)a;
    const IntNode* nb = (const IntNode*)b;
    return (na->value > nb->value) - (na->value < nb->value);
}

// For search operations (key to node comparison)
static int search_cmp(const void* key, const struct bintree* node)
{
    const int* key_val = (const int*)key;
    const IntNode* n = (const IntNode*)node;
    return (*key_val > n->value) - (*key_val < n->value);
}

//───────────────────────────────────────────────
// Helpers
//───────────────────────────────────────────────

static IntNode* make_int_node(int value)
{
    IntNode* n = malloc(sizeof(IntNode));
    if (!n) return NULL;
    
    // Initialize intrusive node
    n->node.btree.parent = NULL;
    n->node.btree.left = NULL;
    n->node.btree.right = NULL;
    
    n->value = value;
    return n;
}

static void int_node_deallocator(void* gnode)
{
    struct bintree* node = gnode;
    if (node == NULL) return;
    IntNode* n = (IntNode*)node;
    free(n);
}

struct object_concept oc = {
    .init = NULL,
    .deinit = int_node_deallocator
};

// Print status progress bar
static void print_progress(size_t current, size_t total)
{
    if (total > 10 && current % (total / 10) == 0) {
        printf("... %zu%%", (current * 100) / total);
        fflush(stdout);
    }
}

//───────────────────────────────────────────────
// Main Test
//───────────────────────────────────────────────

int main(void)
{
    printf("=== AVL INTRUSIVE STRESS TEST START (Size: %d) ===\n", STRESS_COUNT);
    srand((unsigned int)time(NULL));

    // Create Tree
    struct avl tree;
    avl_init(&tree, node_cmp);

    //───────────────────────────────────────────────
    // A. Insertion Phase
    //───────────────────────────────────────────────
    printf("\n[1/4] Inserting %d random integers...", STRESS_COUNT);
    
    // Shadow array to track what was successfully inserted
    int* shadow_data = malloc(STRESS_COUNT * sizeof(int));
    if (!shadow_data) {
        fprintf(stderr, "Failed to allocate shadow array.\n");
        avl_deinit(&tree, &oc);
        return 1;
    }
    
    size_t added_count = 0;

    for (int i = 0; i < STRESS_COUNT; ++i) {
        int val = rand(); 
        
        IntNode* node = make_int_node(val);
        if (!node) {
            fprintf(stderr, "\nFailed to allocate node at iteration %d.\n", i);
            break;
        }

        int status = avl_add(&tree, &node->node);
        
        if (status == 0) {  // TREES_OK
            shadow_data[added_count++] = val;
        } else if (status == 1) {  // TREES_DUPLICATE_KEY
            // Duplicate: free the unused node immediately
            free(node);
        } else {
            // System error
            free(node);
            fprintf(stderr, "\nTree add failed at iteration %d.\n", i);
            break;
        }
        
        print_progress(i, STRESS_COUNT);
    }
    
    printf("\n      -> Requested: %d, Accepted (Unique): %zu", STRESS_COUNT, added_count);
    printf("\n      -> AVL Size Reported: %zu", avl_size(&tree));
    
    // Validate size
    assert(avl_size(&tree) == added_count);
    printf(" [OK]\n");

    //───────────────────────────────────────────────
    // B. Search Phase
    //───────────────────────────────────────────────
    printf("\n[2/4] Verifying all inserted keys exist...");
    for (size_t i = 0; i < added_count; ++i) {
        int key = shadow_data[i];
        struct avl_node* found = avl_search(&tree, &key, search_cmp);
        assert(found != NULL);
        IntNode* found_node = (IntNode*)found;
        assert(found_node->value == key);
        print_progress(i, added_count);
    }
    printf(" [OK]\n");

    //───────────────────────────────────────────────
    // C. Removal Phase (Remove 50% of items)
    //───────────────────────────────────────────────
    size_t remove_target = added_count / 2;
    printf("\n[3/4] Removing %zu items...", remove_target);

    for (size_t i = 0; i < remove_target; ++i) {
        int key = shadow_data[i];

        // Search for the node first
        struct avl_node* node = avl_search(&tree, &key, search_cmp);
        assert(node != NULL);
        
        IntNode* int_node = (IntNode*)node;
        assert(int_node->value == key);

        // Remove the node
        avl_remove(&tree, node);
        
        // Free the user data manually
        free(int_node);
        
        print_progress(i, remove_target);
    }
    
    printf("\n      -> AVL Size Reported: %zu", avl_size(&tree));
    
    assert(avl_size(&tree) == (added_count - remove_target));
    printf(" [OK]\n");

    // Verify removed items are gone
    printf("      -> verifying removed items are gone...");
    for (size_t i = 0; i < remove_target; ++i) {
        int key = shadow_data[i];
        assert(avl_search(&tree, &key, search_cmp) == NULL);
    }
    printf(" [OK]\n");

    // Verify remaining items are still there
    printf("      -> verifying remaining items exist...");
    for (size_t i = remove_target; i < added_count; ++i) {
        int key = shadow_data[i];
        struct avl_node* found = avl_search(&tree, &key, search_cmp);
        assert(found != NULL);
        IntNode* found_node = (IntNode*)found;
        assert(found_node->value == key);
    }
    printf(" [OK]\n");

    //───────────────────────────────────────────────
    // D. Cleanup Phase
    //───────────────────────────────────────────────
    printf("\n[4/4] Destroying tree (cleaning up remaining %zu nodes)...", avl_size(&tree));
    
    // This will iterate remaining nodes and call oc.deinit on each node
    avl_deinit(&tree, &oc);
    
    free(shadow_data);
    printf(" [OK]\n");

    printf("\n=== AVL INTRUSIVE STRESS TEST COMPLETED SUCCESSFULLY ===\n");
    return 0;
}