#ifndef TREES_AVL_H
#define TREES_AVL_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/macros.h>
#include "bintree.h"
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
 * - **AVL Invariants**: You should promise obtained struct bintree * or even
 * - struct avl_node * pointers will not update its data and its parent and child
 * - pointers. These are already managed by the functions here. (this is classic restriction i think)
 * - **NULL Pointers**: All `struct avl *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by you, since nodes are intrusive.
 * - YOU KNOW HOW YOU MANAGE YOUR DATA, so you should what to pass into .deinit method
 * - @ref avl_deinit might be helpful to destruct remaining objects in the tree.
 * @{
 */

/**
 * @brief Safe cast from avl to bintree.
 * 
 * @details
 * Ensures the resulting pointer is treated as const to prevent invariant breaking.
 * Typically used on @ref avl_search results.
 */
#define AVL_TO_BINTREE_CONST(avl_node_ptr) \
    ((const struct bintree *)(avl_node_ptr))

/**
 * @struct avl_node
 * @brief AVL node inheriting from bintree.
 * @warning The 'parent' field in the underlying btree struct is TAGGED.
 * The two least significant bits store the balance factor. 
 * NEVER modify btree.parent or btree.left/right manually.
 * @warning You should not update your nodes data as long as the node
 * is owned and managed by the tree (This was the case in old implementation,
 * since you owned void * pointers to be able to change data anyways).
 */
struct avl_node {
    struct bintree      btree;      ///< Inheriting from bintree, since avl is conceptually a binary tree.
};

/**
 * @struct avl
 * @brief Aggregation of generic binary tree.
 */
struct avl {
    struct avl_node     *root;      ///< Root of the tree.
    size_t              size;       ///< Count of the objects whose references are stored here.
    bst_cmp_cb          cmp;        ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b.
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Initializes the avl.
 * @param[in, out] tree Pointer to avl instance to init.
 * @param[in] cmp Function pointer to compare nodes.
 */
void avl_init(struct avl *tree, bst_cmp_cb cmp);

/**
 * @brief Deinitializes the avl.
 * @param[in] oc object_concept to deinit data references.
 * Must be non-NULL and valid, .deinit too, since you deliberately
 * choosing this function to manage your own memory.
 * @warning Same warning with bintree module here. Since this
 * module uses intrusive nodes, you should be aware that this function
 * passes avl_node into .deinit method, which you supllied, it is entirely
 * up to you how to use that passed node.
 * @warning Only passed trees root attributes and size are set to NULL.
 * @see bintree_deinit
 */
void avl_deinit(struct avl *tree, struct object_concept *oc);

/** @} */ // End of Initialize & Deinitialize

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new node into the avl.
 * @param[in] new_node Hook to the new node.
 * @return 0 if success, 1 if duplicate.
 */
int avl_add(struct avl *btree, struct avl_node *new_node);

/**
 * @brief Removes node from the avl.
 * @param[in] node Node to be removed.
 */
void avl_remove(struct avl *btree, struct avl_node *node);

/**
 * @brief Searches a given data in the avl.
 * @param[in] data Data that is going to be searched.
 * @return struct avl_node * (hook to your data) or NULL if it doesnt exist.
 */
struct avl_node *avl_search(struct avl *btree, const void *data, bintree_cmp_cb cmp);

/** @} */ // End of Operations

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

/** @return Pointer to the root of the avl.
 * @warning SEE GLOBAL CONTRAINTS first. Returning const
 * struct bintree * to reuse bintree iterators and traversal
 * functions. However, they use mutable pointers. So, as a reminder
 * to myself, i dont return mutable pointers to feel the pain of casting
 * and remember avl invariants should be respected.
*/
static inline const struct bintree *avl_root(const struct avl *tree)
{
    assert(tree != NULL);
    return (const struct bintree*) tree->root;
}

/** @return 1 if empty, 0 otherwise. */
static inline int avl_empty(const struct avl *tree)
{
    assert(tree != NULL);
    return tree->size == 0;
}

/** @return size of the avl */
static inline size_t avl_size(const struct avl *tree)
{
    assert(tree != NULL);
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