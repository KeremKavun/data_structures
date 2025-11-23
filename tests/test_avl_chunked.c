#include "../include/avl.h"
#include "../../concepts/include/object_concept.h"
#include "../../allocators/include/chunked_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//=========================
// Compare function
//=========================
static int cmp_int(const void* key, const void* data)
{
    int a = *(const int*)key;
    int b = *(const int*)data;

    if (a < b) return -1;
    if (a > b) return +1;
    return 0;
}

//=========================
// Handler for tree walk
//=========================
static void print_int(void* data, void* userdata)
{
    (void)userdata;
    printf("%d ", *(int*)data);
}

//=========================
// Destructor for the user data
//=========================
static void int_destruct(void* data, void* context)
{
    (void)context;
    free(data);
}

//=========================
// MAIN TEST
//=========================
int main(void)
{
    printf("=== AVL TEST START ===\n");

    //=========================
    // Create chunked_pool allocator for AVL nodes
    //=========================
    size_t objsize = avl_node_sizeof();
    size_t capacity = 128;

    chunked_pool_t* pool = chunked_pool_create(capacity, objsize);
    if (!pool)
    {
        printf("FAILED to create pool\n");
        return 1;
    }

    // object_concept used ONLY for AVL nodes
    struct object_concept oc = {
        .allocator = pool,
        .alloc     = (GENERIC_ALLOC_SIGN) chunked_pool_alloc,
        .free      = (GENERIC_FREE_SIGN) chunked_pool_free,
        .destruct  = int_destruct // <-- FREE THE INTS
    };

    //=========================
    // Create AVL tree
    //=========================
    avl_t* tree = avl_create(cmp_int, &oc);
    if (!tree)
    {
        printf("FAILED to create AVL\n");
        chunked_pool_destroy(pool);
        return 1;
    }

    //=========================
    // INSERT TEST DATA
    //=========================
    printf("\nInserting values:\n");

    int values_to_insert[] = {
        50, 30, 70, 20, 40, 60, 80,
        10, 35, 45, 65, 75, 85,
        5,  15, 25, 33, 37, 43
    };

    size_t nvals = sizeof(values_to_insert) / sizeof(values_to_insert[0]);

    for (size_t i = 0; i < nvals; i++)
    {
        // allocate INT with malloc, not pool
        int* x = malloc(sizeof(int));
        *x = values_to_insert[i];

        enum trees_status st = avl_add(tree, x);
        if (st != TREES_OK)
        {
            printf("Insert failed: %d\n", *x);
            free(x); // avoid leaks
            return 1;
        }
    }

    printf("Tree size after insert: %zu\n", avl_size(tree));

    //=========================
    // SEARCH TEST
    //=========================
    printf("\nSearching some keys:\n");

    int keys[] = {37, 80, 999, 10};
    for (size_t i = 0; i < 4; i++)
    {
        int k = keys[i];
        int* res = avl_search(tree, &k);

        if (res)
            printf("Search %d → found %d\n", k, *res);
        else
            printf("Search %d → NOT FOUND\n", k);
    }

    //=========================
    // INORDER WALK
    //=========================
    printf("\nInorder traversal after insert:\n");
    avl_walk(tree, NULL, print_int, INORDER);
    printf("\n");

    //=========================
    // DELETE TEST
    //=========================
    int values_to_remove[] = {50, 30, 70, 20, 40, 85, 5, 35};
    size_t nrem = sizeof(values_to_remove) / sizeof(values_to_remove[0]);

    printf("\nDeleting values:\n");
    for (size_t i = 0; i < nrem; i++)
    {
        int key = values_to_remove[i];

        enum trees_status st = avl_remove(tree, &key);
        printf("remove(%d) → %s\n", key, st == TREES_OK ? "OK" : "NOT_FOUND");
    }

    printf("\nTree size after deletions: %zu\n", avl_size(tree));

    printf("\nInorder traversal after delete:\n");
    avl_walk(tree, NULL, print_int, INORDER);
    printf("\n");

    //=========================
    // CLEANUP
    //=========================
    avl_destroy(tree, NULL);  // <-- automatically calls int_destruct for every remaining node
    chunked_pool_destroy(pool);

    printf("\n=== AVL TEST END ===\n");
    return 0;
}
