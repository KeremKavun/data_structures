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
size_t bintree_sizeof();

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

struct bintree* bintree_left(const struct bintree* tree);
struct bintree* bintree_right(const struct bintree* tree);
void* bintree_data(struct bintree* tree);

/*───────────────────────────────────────────────
 * Search
 *───────────────────────────────────────────────*/

struct bintree** bintree_search(struct bintree** tree, const void* data, int (*cmp) (const void* key, const void* data));
struct bintree** bintree_findmin(struct bintree** node_ref);
struct bintree** bintree_findmax(struct bintree** node_ref);

/*───────────────────────────────────────────────
 * Traversals
 *───────────────────────────────────────────────*/

void bintree_walk(struct bintree* tree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order);

/*───────────────────────────────────────────────
 * Properties
 *───────────────────────────────────────────────*/

size_t bintree_size(const struct bintree* tree);
int bintree_height(const struct bintree* tree);
int bintree_balance_factor(const struct bintree* tree);

#ifdef __cplusplus
}
#endif

#endif // BIN_TREE_H