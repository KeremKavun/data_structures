#include <ds/trees/bst.h>
#include <stdlib.h>
#include <assert.h>

/* =========================================================================
 * Initialize & Deinitialize
 * ========================================================================= */

void bst_init(struct bst *tree, bst_cmp_cb cmp)
{   
    assert(tree != NULL && cmp != NULL);
    tree->root = NULL;
    tree->cmp = cmp;
    tree->size = 0;
}

void bst_deinit(struct bst *tree, struct object_concept *oc) {
    if (tree->root) {
        bintree_deinit(tree->root, oc);
        tree->root = NULL;
    }
    tree->size = 0;
}

/* =========================================================================
 * Operations
 * ========================================================================= */

int bst_add(struct bst *tree, struct bintree *new_node)
{
    assert(tree != NULL);
    struct bintree *parent;
    struct bintree **link = bintree_search_parent(&tree->root, new_node, &parent, tree->cmp);
    if (*link) {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return 1;
    }
    *link = new_node;
    bintree_init(new_node, parent, NULL, NULL);
    tree->size++;
    return 0;
}

void bst_remove(struct bst *tree, struct bintree *node)
{
    assert(tree != NULL && node != NULL);
    if (node->left && node->right) {
        struct bintree *successor = node->right;
        while (successor->left)
            successor = successor->left;
        bintree_swap(node, successor); 
    }
    struct bintree *child = node->left ? node->left : node->right;
    struct bintree *parent = node->parent;
    if (child)
        child->parent = parent;
    if (!parent) {
        tree->root = child;
    } else {
        if (parent->left == node)
            parent->left = child;
        else
            parent->right = child;
    }
    bintree_init(node, NULL, NULL, NULL);
    tree->size--;
}

struct bintree *bst_search(struct bst *tree, const void *data, bintree_cmp_cb cmp)
{
    assert(tree != NULL);
    return bintree_search(tree->root, data, cmp);
}

/* =========================================================================
 * Inspection
 * ========================================================================= */