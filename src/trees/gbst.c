#include "gbst.h"
#include <assert.h>

struct bintree** gbst_search(struct bintree **tree, const void *data, int (*cmp) (const void *key, const void *data))
{
    assert(tree != NULL);
    struct bintree **curr = tree;
    while (*curr) {
        int result = cmp(data, bintree_get_data(*curr));
        if (result < 0) {
            curr = &(*curr)->left;
        } else if (result > 0) {
            curr = &(*curr)->right;
        } else {
            return curr;
        }
    }
    return curr;
}