#ifndef TREES_AVL_H
#define TREES_AVL_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../include/bintree.h"
#include "../internals/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup AVL API
 * 
 * @brief Basic operations for avl tree.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct avl *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user, stored in the avl,
 * - void *references to data are entirely owned by user. @ref avl_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct avl
 * 
 * @brief Aggregation of generic binary tree.
 */
struct avl {
    struct avl_node             *root;
    struct allocator_concept    ac;
    int (*cmp) (const void *key, const void *data);
    size_t                      size;
};

/**
 * @name Create & Destroy
 * Functions for setting up the tree.
 * @{
 */

// Creates avl and returns, NULL in case of error, if capacity_of_pool is 1, using malloc, else chunked_pool
struct avl *avl_create(int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);
void avl_destroy(struct avl *btree, struct object_concept *oc);
size_t avl_node_sizeof();

/** @} */ // End of Create & Destroy group

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

enum trees_status avl_add(struct avl *btree, void *new_data);
enum trees_status avl_remove(struct avl *btree, void *data);
void *avl_search(struct avl *btree, const void *data);

/** @} */ // End of Operations group

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

static inline const struct bintree *avl_root(const struct avl *tree)
{
    return (const struct bintree*) tree->root;
}

static inline int avl_empty(const struct avl *tree)
{
    return tree->root == NULL;
}

static inline size_t avl_size(const struct avl *tree)
{
    return tree->size;
}

/** @} */ // End of Inspection group

/**
 * @name Traversal
 * Functions for traversals.
 * @{
 */

// Use generic binary tree algorithms, since this is aggregate
// and technically inherits.

/** @} */ // End of Traversal group

#ifdef __cplusplus
}
#endif

#endif // TREES_AVL_H