#ifndef BIN_TREE_H
#define BIN_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../internals/traversals.h"
#include <stddef.h>

struct bintree
{
    struct bintree* left;
    struct bintree* right;
    void* data;
};

typedef struct bintree bintree_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// Creates bintree and returns, NULL in case of error, if capacity_of_pool is 1, using malloc, else chunked_pool
struct bintree* bintree_create(struct object_concept* oc);
void bintree_destroy(struct bintree* tree, void* context, struct object_concept* oc);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

struct bintree** bintree_left(struct bintree* tree);
struct bintree** bintree_right(struct bintree* tree);
void** bintree_data(struct bintree* tree);

/*───────────────────────────────────────────────
 * Traversals
 *───────────────────────────────────────────────*/

void bintree_walk(struct bintree* tree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order);

#ifdef __cplusplus
}
#endif

#endif // BIN_TREE_H