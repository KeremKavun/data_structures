#include "../internals/binsearch_tree.h"

struct bintree** bintree_search(struct bintree** tree, const void* data, int (*cmp) (const void* key, const void* data))
{
    struct bintree** curr = tree;
    while (*curr)
    {
        int result = cmp(data, bintree_get_data(*curr));
        if (result < 0)
            curr = &(*curr)->left;
        else if (result > 0)
            curr = &(*curr)->right;
        else
            return curr;
    }
    return curr;
}