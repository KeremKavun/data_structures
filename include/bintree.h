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
struct bintree* bintree_create(struct bintree* left, struct bintree* right, void* data, struct object_concept* oc);
void bintree_destroy(struct bintree* tree, void* context, struct object_concept* oc);
size_t bintree_sizeof();

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

struct bintree* bintree_get_left(struct bintree* tree);
const struct bintree* bintree_get_left_const(const struct bintree* tree);
void bintree_set_left(struct bintree* tree, struct bintree* left);
struct bintree* bintree_get_right(struct bintree* tree);
const struct bintree* bintree_get_right_const(const struct bintree* tree);
void bintree_set_right(struct bintree* tree, struct bintree* right);
void* bintree_get_data(struct bintree* tree);
const void* bintree_get_data_const(const struct bintree* tree);
void bintree_set_data(struct bintree* tree, void* data);

/*───────────────────────────────────────────────
 * Traversals
 *───────────────────────────────────────────────*/

void bintree_walk(struct bintree* tree, void* userdata, void (*handler) (void* data, void* userdata), enum traversal_order order);

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