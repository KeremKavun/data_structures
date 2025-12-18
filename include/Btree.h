#ifndef BTREE_H
#define BTREE_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../internals/traversals.h"
#include "../internals/status.h"
#include "../include/mwaytree.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Btree;
typedef struct Btree Btree_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct Btree* Btree_create(size_t order, int (*cmp) (const void* key, const void* data), struct object_concept* oc);
void Btree_destroy(struct Btree* tree, void* context);
// SHUT THE FUCK UP! classic mway tree size + one size_t field.
size_t Btree_node_sizeof(size_t order);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

// Add item into the B-tree, classic return types in status.h 
// Unfortunately, i cant provide strong guarantee here, in case of any failure, tree will be fucked up, not just here but in helpers
enum trees_status Btree_add(struct Btree* tree, void* new_data);
// Remove item from the B-tree and return
void* Btree_remove(struct Btree* tree, void* data);
void* Btree_search(struct Btree* tree, const void* data);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const struct mway_header* Btree_root(const struct Btree* tree);
int Btree_empty(const struct Btree* tree);
size_t Btree_size(const struct Btree* tree);
size_t Btree_order(const struct Btree* tree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

// Traverse the B-tree in only inorder
void Btree_walk(struct Btree* tree, void* context, void (*handler) (void* data, void* context));

#ifdef __cplusplus
}
#endif

#endif // BTREE_H