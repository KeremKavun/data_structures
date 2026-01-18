#ifndef TREES_BINTREE_H
#define TREES_BINTREE_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include "common/traversals.h"
#include <ds/utils/macros.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file bintree.h
 * @brief Defines the interface for basic binary tree.
 */

/**
 * @defgroup BINTREE_CORE Binary Trees
 * @ingroup TREES
 * @brief Collection of binary trees (bst, avl, heap)
 */

/**
 * @defgroup BINTREE Basic Binary Tree
 * @ingroup BINTREE_CORE
 * @brief Basic operations for the generic binary tree.
 * 
 * @details
 * ### Global Constraints
 * - **Binary Tree Invariants**: This API exposes struct definitions and provides
 * - many setters to set things, but it is your responsibility to make sure you
 * - dont set parent of a node it node's child. Note that this was same in previous
 * - API, purpose of this API is to reuse this in AVL trees and Red-Black trees in the
 * - future.
 * - **NULL Pointers**: All `struct bintree *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by the user, decided to make all trees
 * - intrusive. This will eliminate allocator_concept usage in the libraries and might
 * - improve cache locality, also adding parent pointers by eliminating void * is huge
 * - win i think, we can implement iterators without wasting memory more. However, this
 * - makes user a bit more responsible about memory management.
 * @{
 */

/**
 * @struct bintree
 * @brief Simple binary tree with intrusive nodes.
 */
struct bintree {
    struct bintree      *parent;        ///< Parent node.
    struct bintree      *left;          ///< Left node.
    struct bintree      *right;         ///< Right node.
};

/**
 * @name Init
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Initializes the binary tree.
 * @param[in] parent Parent of the tree.
 * @param[in] left Left child of the tree.
 * @param[in] right Right child of the tree.
 */
void bintree_init(struct bintree *tree, struct bintree *parent, struct bintree *left, struct bintree *right);

/**
 * @brief Deinits the binary tree.
 * @param[in, out] tree Pointer to the binary tree instance.
 * @param[in] oc object_concept to deinit data references. Might be NULL
 * .deinit method might be NULL, .init isnt used.
 * @warning This is just a utility for specific cases. This function performs
 * postorder traversal, but maybe you manage your nodes in an allocator. Then
 * freeing that allocator would be much faster. Also, if you allocated nodes
 * on the stack and every member of the node is POD, then you should never call this.
 * @warning struct object_concept *oc and its .deinit method should be valid.
 * @warning Only passed trees attributes are set to NULL.
 * @note .deinit method of object takes bintree node.
 */
void bintree_deinit(struct bintree *tree, struct object_concept *oc);

/** @} */ // End of Create & Destroy

/**
 * @name Getters & Setters
 * Functions for get and set things.
 * @{
 */

/** @brief Use this or container_of to get the address of the acutal data. */
#define bintree_get_entry(ptr, type, member) \
    container_of(ptr, type, member)

/** @return left child. */
static inline struct bintree* bintree_get_left(struct bintree* tree)
{
    assert(tree != NULL);
    return tree->left;
}

/** @return const left child. */
static inline const struct bintree* bintree_get_left_const(const struct bintree* tree)
{
    assert(tree != NULL);
    return tree->left;
}

/** @brief Sets left child. */
static inline void bintree_set_left(struct bintree* parent, struct bintree* child)
{
    assert(parent != NULL);
    parent->left = child;
    if (child)
        child->parent = parent;
}

/** @return right child. */
static inline struct bintree* bintree_get_right(struct bintree* tree)
{
    assert(tree != NULL);
    return tree->right;
}

/** @return const right child. */
static inline const struct bintree* bintree_get_right_const(const struct bintree* tree)
{
    assert(tree != NULL);
    return tree->right;
}

/** @brief Sets right child. */
static inline void bintree_set_right(struct bintree* parent, struct bintree* child)
{
    assert(parent != NULL);
    parent->right = child;
    if (child)
        child->parent = parent;
}

/** @return Parent but note that bintrees last 2 bits ar masked. */
static inline struct bintree* bintree_get_parent(struct bintree* tree)
{
    assert(tree != NULL);
    // Adding extra instruction sadly, but my justfication is that
    // this module will be used in red-black tree and avl trees only
    // both of which uses tagged pointers and this module does not have
    // real life use case.
    return (struct bintree *) ((uintptr_t) tree->parent & ~BINTREE_TAG_MASK);
}

/** @return const parent but note that bintrees last 2 bits ar masked. */
static inline const struct bintree* bintree_get_parent_const(const struct bintree* tree)
{
    assert(tree != NULL);
    return (const struct bintree *) ((uintptr_t) tree->parent & ~BINTREE_TAG_MASK);
}

/** @brief Sets parent while PRESERVING existing low-order bits (tags). */
static inline void bintree_set_parent(struct bintree* tree, struct bintree* parent)
{
    assert(tree != NULL);
    uintptr_t existing_tag = (uintptr_t)tree->parent & BINTREE_TAG_MASK;
    tree->parent = (struct bintree *)((uintptr_t)parent | existing_tag);
}

/** @return The root node of the tree containing this node. */
struct bintree *bintree_get_root(struct bintree *node);

/** @return True if node is root (parent == NULL) */
static inline int bintree_is_root(const struct bintree *node)
{
    assert(node != NULL);
    return node->parent == NULL;
}

/** @} */ // End of Getters & Setters

/**
 * @name Subtree Ops
 * @{
 */

/** @brief Detaches subtree from parent, if the node is not the root. */
void bintree_detach(struct bintree *node);

/** @brief Replaces old_node with new_node in tree structure. */
void bintree_replace(struct bintree *old_node, struct bintree *new_node);

/** @brief Swaps two nodes. */
void bintree_swap(struct bintree *n1, struct bintree *n2);

/** @} */ // End of Subtree Ops

/**
 * @name Search
 * @{
 */

/** @brief Commpares two nodes. */
typedef int (*bst_cmp_cb)(const struct bintree *a, const struct bintree *b);

/** @brief Compares desired key with bintree node.
 * 
 * @details The user should retrieve actual data in the
 * node using container_of.
 */
typedef int (*bintree_cmp_cb)(const void *key, const struct bintree *node);

/**
 * @brief Generic search using comparison function.
 * @return struct bintree * (hook to your data).
 * @warning cmp should be valid.
*/
struct bintree *bintree_search(struct bintree *tree, const void *key, bintree_cmp_cb cmp);

/**
 * @brief Searches given nodes parent.
 * @param[in] tree Pointer to tree.
 * @param[in] node Used in cmp to compare.
 * @param[in, out] Used to store parent pointer.
 * @return Pointer to pointer that points to node that
 * stores same data (determined by passed cmp function)
 * as node parameter.
 * @details
 * This function seems stupid but i need this to locate nodes in
 * _add function.
 */
struct bintree **bintree_search_parent(struct bintree **tree, const struct bintree *node, struct bintree **parent, bst_cmp_cb cmp);

/** @} */ // End of Search

/**
 * @name Traversal
 * Functions for traverse binary trees.
 * @{
 */

/** @return Starting point of preorder traversal. */
static struct bintree *bintree_first_preorder(struct bintree *node)
{
    assert(node != NULL);
    return node;
}

/** @return Starting point of inorder traversal. */
struct bintree *bintree_first_inorder(struct bintree *node);

/** @return Starting point of postorder traversal. */
struct bintree *bintree_first_postorder(struct bintree *node);

/** @return Previous node in preorder order. */
struct bintree *bintree_preorder_prev(struct bintree *node);

/** @return Next node in preorder order. */
struct bintree *bintree_preorder_next(struct bintree *node);

/** @return Previous node in inorder order. */
struct bintree *bintree_inorder_prev(struct bintree *node);

/** @return Next node in inorder order. */
struct bintree *bintree_inorder_next(struct bintree *node);

/** @return Previous node in postorder order. */
struct bintree *bintree_postorder_prev(struct bintree *node);

/** @return Next node in postorder order. */
struct bintree *bintree_postorder_next(struct bintree *node);

/** @brief Called in traversal functions. */
typedef void (*bintree_handle_cb) (struct bintree *node, void *context);

/**
 * @brief Traverses tree in the given order
 * @param[in] context Context pointer for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking pointer to node and context pointer. It is callers responsibility
 * to define proper function to interact with actual data stored in the user
 * defined struct where recommendedly last field is struct bintree hook.
 * container_of macro might be used to retrive original node address from bintree
 * node in handler function.
 * @param[in] order Defines the traversel order,
 * preorder, inorder or postorder?
 */
void bintree_traverse(struct bintree *tree, void *context, bintree_handle_cb handler, enum traversal_order order);

/** @brief Traverses the tree in BFS */
void bintree_bfs(struct bintree *tree, void *context, bintree_handle_cb handler);

/** @brief Traverses the tree in DFS */
void bintree_dfs(struct bintree *tree, void *context, bintree_handle_cb handler);

/** @} */ // End of Traversal

/**
 * @name Properties
 * Functions for additional information about the
 * tree, but they might be costful and heavy.
 * @{
 */

/** @return COunt of nodes under this tree */
size_t bintree_size(const struct bintree *tree);

/**
 * @param[in, out] root Optionally to get root pointer.
 * @return Level of node (distance from root)
 */
size_t bintree_level(struct bintree *node, struct bintree **root);

/** @return Height of the tree (NULL tree returns -1 by definition) */
int bintree_height(const struct bintree *tree);

/** @return Left child height - right child height */
int bintree_balance_factor(const struct bintree *tree);

/** @} */ // End of Properties

/** @} */ // End of BINTREE group

#ifdef __cplusplus
}
#endif

#endif // TREES_BINTREE_H