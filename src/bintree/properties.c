#include "../../include/bintree.h"
#include "../../../queue/include/lqueue.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

static void bintree_size_helper(void* item, void* userdata);

size_t bintree_size(const struct bintree* tree)
{
    assert(tree != NULL);
    size_t count = 0;
    bintree_walk((struct bintree*) tree, &count, bintree_size_helper, INORDER);
    return count;
}

int bintree_height(const struct bintree* tree)
{
    assert(tree != NULL);
    int depth = 0;
    struct lqueue lq;
    lqueue_init(&lq, NULL);
    lenqueue(&lq, (struct bintree*) tree);
    while (!lqueue_empty(&lq)) {
        size_t level_size = lqueue_size(&lq);
        for (size_t i = 0; i < level_size; i++) {
            struct bintree* curr = lqueue_front(&lq);
            ldequeue(&lq); 
            if (curr->left)
                lenqueue(&lq, curr->left);
            if (curr->right)
                lenqueue(&lq, curr->right);
        }
        depth++;
    }
    return depth - 1;
}

int bintree_balance_factor(const struct bintree* tree)
{
    assert(tree != NULL);
    return bintree_height(tree->left) - bintree_height(tree->right);
}

// *** Helper functions *** //

static void bintree_size_helper(void* item, void* userdata)
{
    (void) item;
    size_t* count = userdata;
    count++;
}