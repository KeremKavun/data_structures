#ifndef TREES_GBST_H
#define TREES_GBST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../include/bintree.h"

struct bintree** gbst_search(struct bintree **tree, const void *data, int (*cmp) (const void *key, const void *data));

#ifdef __cplusplus
}
#endif

#endif // TREES_GBST_H