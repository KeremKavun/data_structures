#ifndef TREES_AVL_H
#define TREES_AVL_H

#include <ds/utils/debug.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include "bintree.h"
#include "common/status.h"
#include <stddef.h>
#include <stdint.h>

#define AVL_NODE(ptr) ((struct avl_node *)(ptr))
#define BIN_NODE(ptr) ((struct bintree *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file avl.h
 * @brief Defines the interface for AVL trees.
 */

/**
 * @defgroup AVL Avl Tree
 * @ingroup BINTREE_CORE
 * @brief Basic operations for avl tree.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct avl *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user, stored in the avl,
 * - void *references to data are entirely owned by user. @ref avl_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct avl
 * @brief Aggregation of generic binary tree.
 */
struct avl {
    struct avl_node             *root;                  ///< Root of the tree.
    struct allocator_concept    ac;                     ///< Used by the tree to allocate new nodes to maintain the tree.
    int (*cmp) (const void *key, const void *data);     ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b. 
    size_t                      size;                   ///< Count of the objects whose references are stored here.
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Initializes the avl.
 * @param[in, out] tree Pointer to avl instance to init.
 * @param[in] cmp Function pointer to compare keys.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * Use @ref avl_node_sizeof() to pass object size into your allocator that will allocate avl_nodes.
 */
void avl_init(struct avl *tree, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);

/**
 * @brief Deinitializes the avl.
 * @param[in] oc object_concept to deinit data references.
 * @warning Only root is set to NULL after freeing the internal tree.
 */
void avl_deinit(struct avl *tree, struct object_concept *oc);

/** @return sizeof(struct avl_node) */
size_t avl_node_sizeof();

/** @} */ // End of Initialize & Deinitialize

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new data into the avl.
 * @param[in] new_data Reference to the new data.
 * @return enum tree_status, which might indicate
 * duplicate or memory allocation failure.
 */
enum trees_status avl_add(struct avl *btree, void *new_data);

/**
 * @brief Removes data from the avl.
 * @param[in] data Data to be removed.
 * @return Data that was stored in the avl or NULL if doesnt exist or if stack depth
 * is exceeded but this is practivcally impossible, since the tree is balanced and the
 * tree with height 64 would store 2^64 - 1 node.
 * @warning **Lifetime Management**: The tree did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back.
 */
void *avl_remove(struct avl *btree, void *data);

/**
 * @brief Searches a given data in the avl.
 * @param[in] data Data that is going to be searched.
 * @return Data that was stored in the avl or NULL if doesnt exist.
 */
void *avl_search(struct avl *btree, const void *data);

/** @} */ // End of Operations

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

/** @return The root of the avl.
 * @warning NEVER remove constness because some bintree functions
 * might not respect avl invariants. This function useful to reuse
 * some bintree functions such as traversals. Also keep in mind that
 * there is risk of confusing actual bintree nodes and casted avl nodes. 
*/
static inline const struct bintree *avl_root(const struct avl *tree)
{
    return (const struct bintree*) tree->root;
}

/** @return 1 if empty, 0 otherwise. */
static inline int avl_empty(const struct avl *tree)
{
    return tree->root == NULL;
}

/** @return size of the avl */
static inline size_t avl_size(const struct avl *tree)
{
    return tree->size;
}

/** @} */ // End of Inspection

/**
 * @name Traversal
 * Functions for traversals.
 * @{
 */

// Use generic binary tree algorithms, since this is aggregate
// and technically inherits.

// Use bintree_left and bintree_right for navigation
// after obtaining root by avl_root, for example.

/** @} */ // End of Traversal

/** @} */ // End of AVL group

#ifdef __cplusplus
}
#endif

#endif // TREES_AVL_H