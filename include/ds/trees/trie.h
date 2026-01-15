#ifndef TREES_TRIE_H
#define TREES_TRIE_H

#include "mwaytree.h"
#include "common/status.h"
#include <ds/utils/allocator_concept.h>
#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file trie.h
 * @brief Defines the interface for a Generic Trie (Prefix Tree).
 */

/**
 * @defgroup TRIE Trie
 * @ingroup MWAYTREE_CORE
 * @brief Dictionary implementation using an M-Way Tree.
 * 
 * @details
 * This Trie implementation maps `const char*` keys to `void*` values.
 * It utilizes the underlying `mwaytree` node structure where the 
 * capacity determines the alphabet size (e.g., 256 for extended ASCII).
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct trie *tr` arguments must be non-NULL.
 * - **Height Limit**: Iterators stack depth will be 32.
 * @{
 */

#define TRIE_STACK_DEPTH 32

/**
 * @brief Configuration callback to map a char to an index.
 * @return Index between 0 and (capacity - 1).
 */
typedef size_t (*trie_map_cb) (unsigned char c);

/**
 * @brief Configuration callback to map an index to a char.
 * @return unsigned char.
 */
typedef unsigned char (*trie_unmap_cb) (size_t i);

/**
 * @struct trie
 * @brief Simple trie.
 * 
 * @details
 * The root field of the struct is mway_entry, because we want to
 * represent empty strings. This can be done just by storing void *
 * and struct mway_hader *, which is what mway_entry do. I am
 * frustrated a bit, because it doesnt sound sematically correct;
 * root is not a node, but i dont know how it would be handled more
 * gracefully. 
 */
struct trie {
    struct mway_entry               root;               ///< The root node of the trie, representing empty string.
    struct allocator_concept        ac;                 ///< Allocator for internal nodes.
    size_t                          alphabet_size;      ///< Capacity (N) for mway nodes.
    size_t                          count;              ///< Count of objects in the dictionary.
    trie_map_cb                     mapper;             ///< Maps char -> index.
    trie_unmap_cb                   unmapper;           ///< UNmaps index -> char.
};

/**
 * @name Init & Deinit
 * @{
 */

/**
 * @brief Initializes a Trie.
 * @param tr Pointer to the trie struct.
 * @param ac Allocator to use for node creation.
 * @param alphabet_size Size of the alphabet (e.g., 256).
 * @param mapper Optional function to map chars to indices,
 * must be valid func ptr.
 * @return 0 if succeeds, non-zero otherwise.
 */
void trie_init(struct trie *tr, struct allocator_concept *ac, size_t alphabet_size, trie_map_cb mapper, trie_unmap_cb unmapper);

/**
 * @brief Destroys the Trie and frees all nodes.
 * @param tr Pointer to the trie.
 * @param oc Optional object_concept to free the `void*` data stored in the trie.
 * @warning This function only sets root.child and root.data to NULL. Remaining fields
 * remain same, i decided this thinking we might not always need to NULLify the object
 * for example, we might just free it and forget it. Any ideas?
 */
void trie_deinit(struct trie *tr, struct object_concept *oc);

static inline size_t trie_node_sizeof(size_t alphabet_size)
{
    return mway_sizeof(alphabet_size, 0);
}


/** @} */ // End of Init & Deinit

/**
 * @name Dictionary Operations
 * @{
 */

/**
 * @brief Inserts or updates a value associated with a string key.
 * @param key Null-terminated string key.
 * @param value Pointer to the data to store.
 * @return The old value if the key already existed, NULL otherwise.
 */
void *trie_put(struct trie *tr, const char *key, void *value);

/**
 * @brief Retrieves a value associated with a string key.
 * @param[in] key Null-terminated string key.
 * @param[in] value Pointer to pointer to store void pointer of value
 * you provided.
 * @return TREES_OK if successful, TREES_UNKNOWN_INPUT if unsanitized
 * input passed, TREES_NOT_FOUND if the key not found in the trie.
 */
enum trees_status trie_get(struct trie *tr, const char *key, void **value);

/**
 * @brief Removes a key from the trie.
 * @param key Null-terminated string key.
 * @param[in] value Pointer to pointer to store void pointer of value
 * you provided.
 * @return TREES_OK if successful, TREES_UNKNOWN_INPUT if unsanitized
 * input passed, TREES_NOT_FOUND if the key not found in the trie.
 * @note This function does not frees node if the last key is removed from the node,
 * will be considered later, probably will try to implement freeing efficiently.
 */
enum trees_status trie_remove(struct trie *tr, const char *key, void **removed);

/** @return Count of the objects in the dictionary. */
static inline size_t trie_size(struct trie *tr)
{
    assert(tr != NULL);
    return tr->count;
}

/** @return 1 if the key is in dictionary, 0 otherwise. */
int trie_contains(struct trie *tr, const char *key);

/** @return 1 if the trie is empty, 0 otherwise. */
static inline int trie_is_empty(struct trie *tr)
{
    assert(tr != NULL);
    return tr->count == 0;
}

/** @} */ // End of Dictionary Operations

/**
 * @name Advanced Operations
 * @{
 */

/**
 * @brief Callback for traversals.
 * @param key The reconstructed key at the current node (buffer provided by iterator).
 * @param value The value stored at this node.
 * @param context Passthrough argument.
 * @return 1 to continue traversal, false to 0.
 */
typedef int (*trie_visit_cb)(const char *key, void *value, void *context);

/**
 * @brief Iterates over all keys starting with the given prefix.
 * @param prefix Null-terminated string prefix (pass "" to iterate all).
 * @param context User data passed to callback.
 * @param cb Callback function invoked for every match.
 */
void trie_prefix_iterate(struct trie *tr, const char *prefix, void *context, trie_visit_cb cb);

/**
 * @brief Finds the longest prefix of `key` that exists in the Trie.
 * @param key The input string.
 * @return The length of the longest matching prefix.
 */
size_t trie_longest_prefix(struct trie *tr, const char *key);

/** @} */ // End of Advanced Operations

/** @} */ // End of TRIE group

#ifdef __cplusplus
}
#endif

#endif // TREES_TRIE_H