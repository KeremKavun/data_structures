#ifndef TREES_AVL_H
#define TREES_AVL_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../include/bintree.h"
#include "../internals/status.h"
#include <stddef.h>
#include <stdint.h>

#define AVL_NODE(ptr) ((struct avl_node *)(ptr))
#define BIN_NODE(ptr) ((struct bintree *)(ptr))

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

struct avl_node;

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

/**
 * @brief Creates the avl.
 * 
 * @param[in] cmp Function pointer to compare keys.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * 
 * @return Avl, NULL if not successful.
 */
struct avl *avl_create(int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);

/**
 * @brief Destroys the avl.
 * 
 * @param[in] oc object_concept to deinit data references.
 */
void avl_destroy(struct avl *btree, struct object_concept *oc);

/**
 * @return sizeof(struct avl_node)
 */
size_t avl_node_sizeof();

/** @} */ // End of Create & Destroy group

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new data into the avl.
 * 
 * @param[in] new_data Reference to the new data.
 * 
 * @return enum tree_status, which might indicate
 * duplicate or memory allocation failure.
 */
enum trees_status avl_add(struct avl *btree, void *new_data);

/**
 * @brief Removes data from the avl.
 * 
 * @param[in] data Data to be removed.
 * 
 * @return Data that was stored in the avl or NULL if doesnt exist.
 * 
 * @warning **Lifetime Management**: The tree did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back.
 */
void *avl_remove(struct avl *btree, void *data);

/**
 * @brief Searches a given data in the avl.
 * 
 * @param[in] data Data that is going to be searched.
 * 
 * @return Data that was stored in the avl or NULL if doesnt exist.
 */
void *avl_search(struct avl *btree, const void *data);

/** @} */ // End of Operations group

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

/**
 * @return The root of the avl.
 */
static inline struct bintree *avl_root(struct avl *tree)
{
    return (struct bintree*) tree->root;
}

/**
 * @return 1 if empty, 0 otherwise.
 */
static inline int avl_empty(const struct avl *tree)
{
    return tree->root == NULL;
}

/**
 * @return size of the avl
 */
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

/**
 * @return prev node in inorder traversal.
 */
struct avl_node *avl_node_prev(struct avl_node *node);

/**
 * @return next node in inorder traversal.
 */
struct avl_node *avl_node_next(struct avl_node *node);

/** @} */ // End of Traversal group

#ifdef __cplusplus
}
#endif

#endif // TREES_AVL_H