#include "../../include/bintree.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

// Helper prototype
static void bintree_size_helper(void* item, void* userdata);

size_t bintree_size(const struct bintree* tree)
{
    if (tree == NULL)
        return 0;
    size_t count = 0;
    bintree_traverse((struct bintree*) tree, &count, bintree_size_helper, INORDER);
    return count;
}

int bintree_height(const struct bintree* tree)
{
    if (tree == NULL)
        return -1; 
    int left_height = bintree_height(tree->left);
    int right_height = bintree_height(tree->right);
    return 1 + (left_height > right_height ? left_height : right_height);
}

int bintree_balance_factor(const struct bintree* tree)
{
    if (tree == NULL)
        return 0;
    return bintree_height(tree->left) - bintree_height(tree->right);
}

// *** Helper functions *** //

static void bintree_size_helper(void* item, void* userdata)
{
    (void) item;
    size_t* count = (size_t*) userdata;
    (*count)++;
}