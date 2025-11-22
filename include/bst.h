#ifndef BST_H
#define BST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../internals/traversals.h"
#include "../internals/status.h"
#include "../internals/binsearch_tree.h"
#include <stddef.h>

struct bst;
typedef struct bst bst_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// Creates bst and returns, NULL in case of error, if capacity_of_pool is 1, using malloc, else chunked_pool
struct bst* bst_create(int (*cmp) (const void* key, const void* data), struct object_concept* oc);
void bst_destroy(struct bst* tree, void* context);
size_t bst_node_sizeof();

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status bst_add(struct bst* tree, void* new_data);
enum trees_status bst_remove(struct bst* tree, void* data);
void* bst_search(struct bst* tree, const void* data);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bst_empty(const struct bst* tree);
size_t bst_size(const struct bst* tree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* tree, void* userdata, void (*handler) (void* data, void* userdata), enum traversal_order order);

#ifdef __cplusplus
}
#endif

#endif // BST_H