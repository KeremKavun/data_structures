#include "../include/bst.h"
#include "../internals/gbst.h"
#include <stdlib.h>
#include <assert.h>

struct bintree **bintree_findmin(struct bintree **node_ref);

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct bst* bst_create(int (*cmp) (const void *key, const void *data), struct allocator_concept *ac)
{   
    struct bst* tree = malloc(sizeof(struct bst));
    if (!tree) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    tree->root = NULL;
    tree->ac = *ac;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}

void bst_destroy(struct bst *tree, struct object_concept *oc)
{
    assert(tree != NULL);
    bintree_destroy(tree->root, oc, &tree->ac);
    free(tree);
}

size_t bst_node_sizeof()
{
    return sizeof(struct bintree);
}

/* =========================================================================
 * Operations
 * ========================================================================= */

enum trees_status bst_add(struct bst *tree, void *new_data)
{
    assert(tree != NULL);
    LOG(LIB_LVL, CINFO, "Adding new data at %p", new_data);
    struct bintree **curr = gbst_search(&tree->root, new_data, tree->cmp);
    if (*curr) {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return TREES_DUPLICATE_KEY;
    }
    struct bintree *new_node = bintree_create(NULL, NULL, new_data, &tree->ac);
    if (!new_node) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for node");
        return TREES_SYSTEM_ERROR;
    }
    *curr = new_node;
    tree->size++;
    return TREES_OK;
}

void *bst_remove(struct bst *tree, void *data)
{
    assert(tree != NULL);
    LOG(LIB_LVL, CINFO, "Removing data at %p", data);
    struct bintree **target = gbst_search(&tree->root, data, tree->cmp);
    if (!(*target)) {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NULL;
    }
    struct bintree *node = *target;
    if (!node->left) {
        *target = node->right;
    } else if (!node->right) {
        *target = node->left;
    } else {
        struct bintree **min = bintree_findmin(&node->right);
        struct bintree *successor = *min;
        *min = successor->right;
        void *tmp = node->data;
        node->data = successor->data;
        successor->data = tmp;
        node = successor;
    }
    void *removed_data = node->data;
    tree->ac.free(tree->ac.allocator, node);
    tree->size--;
    return removed_data;
}

void *bst_search(struct bst* tree, const void *data)
{
    assert(tree != NULL);
    struct bintree **target = gbst_search(&tree->root, data, tree->cmp);
    if (!(*target)) {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NULL;
    }
    return (*target)->data;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

// *** Helper functions *** //

struct bintree **bintree_findmin(struct bintree **node_ref)
{
    if (!node_ref || !*node_ref)
        return NULL;
    struct bintree **curr = node_ref;
    while ((*curr)->left) {
        curr = &(*curr)->left;
    }
    return curr;
}