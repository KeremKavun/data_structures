#include <ds/trees/avl.h>
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define STRESS_COUNT 50000

//───────────────────────────────────────────────
// Helpers
//───────────────────────────────────────────────

static int cmp_int(const void* key, const void* data) {
    const int* a = key;
    const int* b = data;
    return (*a > *b) - (*a < *b);
}

static void int_deinit(void* data) {
    free(data);
}

// Print status progress bar
static void print_progress(size_t current, size_t total) {
    if (total > 10 && current % (total / 10) == 0) {
        printf("... %zu%%", (current * 100) / total);
        fflush(stdout);
    }
}

//───────────────────────────────────────────────
// Main Test
//───────────────────────────────────────────────

int main(void) {
    printf("=== AVL STRESS TEST START (Size: %d) ===\n", STRESS_COUNT);
    srand((unsigned int)time(NULL));

    // 1. Configure Syspool for AVL nodes
    struct syspool pool = { .obj_size = avl_node_sizeof() };

    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc     = (GENERIC_ALLOC_SIGN) sysalloc,
        .free      = (GENERIC_FREE_SIGN) sysfree
    };

    // 2. Configure Object Concept (only destructor for cleanup)
    struct object_concept oc = {
        .init   = NULL,
        .deinit = int_deinit 
    };

    // 3. Create Tree
    struct avl* tree = avl_create(cmp_int, &ac);
    if (!tree) {
        fprintf(stderr, "Failed to create tree.\n");
        return 1;
    }

    //───────────────────────────────────────────────
    // A. Insertion Phase
    //───────────────────────────────────────────────
    printf("\n[1/4] Inserting %d random integers...", STRESS_COUNT);
    
    // Shadow array to track what was successfully inserted
    int* shadow_data = malloc(STRESS_COUNT * sizeof(int));
    if (!shadow_data) {
        fprintf(stderr, "Failed to allocate shadow array.\n");
        avl_destroy(tree, &oc);
        return 1;
    }
    
    size_t added_count = 0;

    for (int i = 0; i < STRESS_COUNT; ++i) {
        int val = rand(); 
        
        // Allocate memory for the tree data (user owns this)
        int* data_ptr = malloc(sizeof(int));
        if (!data_ptr) {
            fprintf(stderr, "\nFailed to allocate data at iteration %d.\n", i);
            break;
        }
        *data_ptr = val;

        enum trees_status st = avl_add(tree, data_ptr);
        
        if (st == TREES_OK) {
            shadow_data[added_count++] = val;
        } else if (st == TREES_DUPLICATE_KEY) {
            // Duplicate: free the unused memory immediately
            free(data_ptr);
        } else {
            // System error
            free(data_ptr);
            fprintf(stderr, "\nTree add failed at iteration %d.\n", i);
            break;
        }
        
        print_progress(i, STRESS_COUNT);
    }
    
    printf("\n      -> Requested: %d, Accepted (Unique): %zu", STRESS_COUNT, added_count);
    printf("\n      -> AVL Size Reported: %zu", avl_size(tree));
    
    // Validate size
    assert(avl_size(tree) == added_count);
    printf(" [OK]\n");

    //───────────────────────────────────────────────
    // B. Search Phase
    //───────────────────────────────────────────────
    printf("\n[2/4] Verifying all inserted keys exist...");
    for (size_t i = 0; i < added_count; ++i) {
        int key = shadow_data[i];
        int* res = avl_search(tree, &key);
        assert(res != NULL && *res == key);
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

        // Remove and get the data pointer back
        int* removed_data = avl_remove(tree, &key);
        assert(removed_data != NULL); // Should be there
        assert(*removed_data == key);

        // Free the user data manually
        free(removed_data);
        
        print_progress(i, remove_target);
    }
    
    printf("\n      -> AVL Size Reported: %zu", avl_size(tree));
    
    assert(avl_size(tree) == (added_count - remove_target));
    printf(" [OK]\n");

    // Verify removed items are gone
    printf("      -> verifying removed items are gone...");
    for (size_t i = 0; i < remove_target; ++i) {
        int key = shadow_data[i];
        assert(avl_search(tree, &key) == NULL);
    }
    printf(" [OK]\n");

    // Verify remaining items are still there
    printf("      -> verifying remaining items exist...");
    for (size_t i = remove_target; i < added_count; ++i) {
        int key = shadow_data[i];
        int* res = avl_search(tree, &key);
        assert(res != NULL && *res == key);
    }
    printf(" [OK]\n");

    //───────────────────────────────────────────────
    // D. Cleanup Phase
    //───────────────────────────────────────────────
    printf("\n[4/4] Destroying tree (cleaning up remaining %zu nodes)...", avl_size(tree));
    
    // This will iterate remaining nodes and call oc.deinit (int_deinit/free) on each data pointer
    avl_destroy(tree, &oc);
    
    free(shadow_data);
    printf(" [OK]\n");

    printf("\n=== AVL STRESS TEST COMPLETED SUCCESSFULLY ===\n");
    return 0;
}