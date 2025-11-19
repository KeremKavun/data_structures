#include "../include/bst.h"
#include <stdlib.h>

struct avl
{
    struct bintree* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

static struct bintree* avl_insert_helper(struct avl* tree, struct bintree* btree, void* new_data);
static void balance_left(struct bintree* tree);
static void balance_right(struct bintree* tree);
static void rotate_left(struct bintree* tree);
static void rotate_right(struct bintree* tree);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct avl* avl_create(int (*cmp) (const void* key, const void* data), struct object_concept* oc)
{
    struct avl* tree = malloc(sizeof(struct avl));
    if (!tree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    tree->root = NULL;
    tree->oc = oc;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}

void avl_destroy(struct avl* tree, void* context)
{
    bintree_destroy(tree->root, context, tree->oc);
    free(tree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status avl_add(struct avl* tree, void* new_data)
{
    
}

enum trees_status avl_remove(struct avl* tree, void* data)
{

}

void* avl_search(struct avl* tree, const void* data)
{

}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int avl_empty(const struct avl* tree)
{

}

size_t avl_size(const struct avl* tree)
{

}

void* avl_min(struct avl* tree)
{

}

void* avl_max(struct avl* tree)
{

}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void avl_walk(struct avl* tree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order)
{

}

// *** Helper functions *** //

static struct bintree* avl_insert_helper(struct avl* tree, struct bintree* btree, void* new_data)
{
    if (!btree)
    {
        struct bintree* new_tree = bintree_create(tree->oc);
        return new_tree;
    }
    if (tree->cmp(btree->data, new_data) > 0)
    {
        avl_insert_helper(tree, btree->left, new_data);
        if (bintree_balance_factor(tree) > 1)
            rotate_right();
    }
    else
    {
        avl_insert_helper(tree, btree->right, new_data);
        if (bintree_balance_factor(tree) < -1)
            rotate_left();
    }
    return tree;
}

static void balance_left(struct bintree* tree)
{
    if (bintree_balance_factor(tree) > 0)
}

static void balance_right(struct bintree* tree)
{

}

static void rotate_left(struct bintree* tree)
{
    struct bintree* tmp = tree->right->left;
    tree->right->left  = tree;
    tree->right = tmp;
}

static void rotate_right(struct bintree* tree)
{
    struct bintree* tmp = tree->left->right;
    tree->left->right = tree;
    tree->left = tmp;
}