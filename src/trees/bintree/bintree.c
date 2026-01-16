#include <ds/trees/bintree.h>
#include <assert.h>

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct bintree *bintree_create(struct bintree *left, struct bintree *right, void *data, struct allocator_concept* ac)
{
    assert(ac != NULL && ac->alloc != NULL && ac->allocator != NULL);
    struct bintree *bintree = ac->alloc(ac->allocator);
    if (!bintree) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for bintree");
        return NULL;
    }
    bintree->left = left;
    bintree->right = right;
    bintree->data = data;
    return bintree;
}

void bintree_destroy(struct bintree *tree, struct object_concept *oc, struct allocator_concept *ac)
{
    assert(ac != NULL && ac->free != NULL && ac->allocator != NULL); // Sacrificing performance in debug mode.
    if (!tree)
        return;
    bintree_destroy(tree->left, oc, ac);
    bintree_destroy(tree->right, oc, ac);
    if (oc && oc->deinit)
        oc->deinit(tree->data);
    ac->free(ac->allocator, tree);
}

/* =========================================================================
 * Getters & Setters
 * ========================================================================= */