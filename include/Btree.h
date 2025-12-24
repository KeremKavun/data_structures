#ifndef BTREE_H
#define BTREE_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../internals/traversals.h"
#include "../internals/status.h"
#include "../include/mwaytree.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup B-Tree API
 * 
 * @brief Basic operations for B-trees.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct Btree *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user, stored in the avl,
 * - void *references to data are entirely owned by user. @ref avl_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct Btree
 */
struct Btree {
    struct mway_header              *root;
    struct allocator_concept        ac;
    int (*cmp) (const void *key, const void *data);
    size_t                          size;
};

/**
 * @name Create & Destroy
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Creates B-tree.
 * 
 * @param[in] order Order of B-tree.
 * @param[in] cmp Function pointer to compare keys.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * 
 * @return Btree instance.
 */
struct Btree *Btree_create(size_t order, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);

/**
 * @brief Destroys the B-tree.
 * 
 * @param[in] oc object_concept to deinit data references.
 */
void Btree_destroy(struct Btree *tree, struct object_concept *oc);

/**
 * @return @ref mway_sizeof(order - 1, sizeof(struct mway_header*) + sizeof(size_t))
 * 
 * @see mway_sizeof
 */
size_t Btree_node_sizeof(size_t order)
{
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    return mway_sizeof(order - 1, sizeof(struct mway_header*) + sizeof(size_t));
}

/** @} */ // End of Create & Destroy group

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

// Add item into the B-tree, classic return types in status.h 
// Unfortunately, i cant provide strong guarantee here, in case of any failure, tree will be fucked up, not just here but in helpers
enum trees_status Btree_add(struct Btree *tree, void *new_data);
// Remove item from the B-tree and return
void *Btree_remove(struct Btree *tree, void *data);
void *Btree_search(struct Btree *tree, const void *data);

/** @} */ // End of Operations group

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

static inline const struct mway_header* Btree_root(const struct Btree* tree)
{
    return tree->root;
}

static inline int Btree_empty(const struct Btree* tree)
{
    return tree->root == NULL;
}

static inline size_t Btree_size(const struct Btree* tree)
{
    return tree->size;
}

static inline size_t Btree_order(const struct Btree* tree)
{
    return tree->root->capacity + 1;
}

/** @} */ // End of Inspection group

/**
 * @name Traversal
 * Functions for traversals.
 * @{
 */

/**
 * @brief Traverses tree in the inorder.
 * 
 * @param[in] context Context pointer for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void Btree_walk(struct Btree *tree, void *context, void (*handler) (void *data, void *context));

/** @} */ // End of Traversal group

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // BTREE_H