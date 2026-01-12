#include "../../include/bintree.h"
#include "../../../queue/include/lqueue.h"
#include "../../../stack/include/lstack.h"
#include <limits.h>
#include <stdlib.h>

static void bintree_walk_subtree(struct bintree *tree, void *context, void (*handler) (void *data, void *context), int *func_index_array);

/* =========================================================================
* Traversals
* ========================================================================= */

void bintree_traverse(struct bintree *tree, void *context, void (*handler) (void *data, void *context), enum traversal_order order)
{
    int func_index_array[3] = {0, 0, 0};
    func_index_array[order] = 1;
    bintree_walk_subtree(tree, context, handler, func_index_array);
}

/*
int bintree_bfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context))
{
}

int bintree_dfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context))
{
}
*/

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
