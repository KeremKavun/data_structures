#include <ds/trees/bintree.h>
#include <ds/queue/lqueue.h>
#include <ds/stack/lstack.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

static void bintree_walk_subtree(struct bintree *tree, void *context, void (*handler) (void *data, void *context), int *func_index_array);

/* =========================================================================
* Traversals
* ========================================================================= */

void bintree_traverse(struct bintree *tree, void *context, void (*handler) (void *data, void *context), enum traversal_order order)
{
    assert(tree != NULL && handler != NULL);
    int func_index_array[3] = {0, 0, 0};
    func_index_array[order] = 1;
    bintree_walk_subtree(tree, context, handler, func_index_array);
}

/**
 * Would it be good practice trying to reuse graph implementation's bfs and dfs?
 * Should i establish a hierarchy between trees and graphs so that conversions from
 * trees to graphs become very easy, since trees are special graphs? I have no idea,
 * currently but will ponder. Gemini says no in short.
 */

int bintree_bfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context))
{
    assert(tree != NULL && handler != NULL);
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lqueue_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lqueue *lq = lqueue_create(&ac);
    struct bintree *curr = tree;
    lenqueue(lq, curr); 
    while (!lqueue_empty(lq)) {
        curr = ldequeue(lq);
        handler(curr->data, context);
        if (curr->left)
            lenqueue(lq, curr->left);
        if (curr->right)
            lenqueue(lq, curr->right);
    }
    lqueue_destroy(lq, NULL);
    return 0;
}

int bintree_dfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context))
{
    assert(tree != NULL && handler != NULL);
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lstack_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lstack *ls = lstack_create(&ac);
    struct bintree *curr = tree;
    lpush(ls, curr);
    while (!lstack_empty(ls)) {
        curr = lpop(ls);
        handler(curr->data, context);
        if (curr->right)
            lpush(ls, curr->right);
        if (curr->left)
            lpush(ls, curr->left);
    }
    lstack_destroy(ls, NULL);
    return 0;
}

// *** Helper functions *** //

static void bintree_walk_subtree(struct bintree *tree, void *context, void (*handler) (void *data, void *context), int *func_index_array)
{
    if (!tree)
        return;
    if (func_index_array[0])
        handler(tree->data, context);
    bintree_walk_subtree(tree->left, context, handler, func_index_array);
    if (func_index_array[1])
        handler(tree->data, context);
    bintree_walk_subtree(tree->right, context, handler, func_index_array);
    if (func_index_array[2])
        handler(tree->data, context);
}
