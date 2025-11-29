#include "../../include/bintree.h"
#include <stdlib.h>

static void bintree_walk_subtree(struct bintree* tree, void* userdata, void (*handler) (void* data, void* userdata), int *func_index_array);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bintree* bintree_create(struct bintree* left, struct bintree* right, void* data, struct object_concept* oc)
{
    struct bintree* bintree = (oc && oc->allocator) ? oc->alloc(oc->allocator) : malloc(sizeof(struct bintree));
    if (!bintree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for bintree");
        return NULL;
    }
    bintree->left = left;
    bintree->right = right;
    bintree->data = data;
    return bintree;
}

void bintree_destroy(struct bintree* tree, void* context, struct object_concept* oc)
{
    if (!tree)
        return;
    bintree_destroy(tree->left, context, oc);
    bintree_destroy(tree->right, context, oc);
    if (oc && oc->destruct)
        oc->destruct(tree->data, context);
    (oc && oc->allocator) ? oc->free(oc->allocator, tree) : free(tree);
}

size_t bintree_sizeof()
{
    return sizeof(struct bintree);
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

struct bintree* bintree_get_left(struct bintree* tree)
{
    return tree->left;
}

const struct bintree* bintree_get_left_const(const struct bintree* tree)
{
    return tree->left;
}

void bintree_set_left(struct bintree* tree, struct bintree* left)
{
    tree->left = left;
}

struct bintree* bintree_get_right(struct bintree* tree)
{
    return tree->right;
}

const struct bintree* bintree_get_right_const(const struct bintree* tree)
{
    return tree->right;
}

void bintree_set_right(struct bintree* tree, struct bintree* right)
{
    tree->right = right;
}

void* bintree_get_data(struct bintree* tree)
{
    return tree->data;
}

const void* bintree_get_data_const(const struct bintree* tree)
{
    return tree->data;
}

void bintree_set_data(struct bintree* tree, void* data)
{
    tree->data = data;
}

/*───────────────────────────────────────────────
 * Traversal
 *───────────────────────────────────────────────*/

void bintree_walk(struct bintree* tree, void* userdata, void (*handler) (void* data, void* userdata), enum traversal_order order)
{
    int func_index_array[3] = {0, 0, 0};
    func_index_array[order] = 1;
    bintree_walk_subtree(tree, userdata, handler, func_index_array);
}

// *** Helper functions *** //

static void bintree_walk_subtree(struct bintree* tree, void* userdata, void (*handler) (void* data, void* userdata), int *func_index_array)
{
    if (!tree)
        return;
    if (func_index_array[0])
        handler(tree->data, userdata);
    bintree_walk_subtree(tree->left, userdata, handler, func_index_array);
    if (func_index_array[1])
        handler(tree->data, userdata);
    bintree_walk_subtree(tree->right, userdata, handler, func_index_array);
    if (func_index_array[2])
        handler(tree->data, userdata);
}