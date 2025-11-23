#ifndef BINSEARCH_TREE_H
#define BINSEARCH_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../include/bintree.h"

struct bintree** bintree_search(struct bintree** tree, const void* data, int (*cmp) (const void* key, const void* data));

#ifdef __cplusplus
}
#endif

#endif // BINSEARCH_TREE_H