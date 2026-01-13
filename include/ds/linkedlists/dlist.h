#ifndef LINKEDLISTS_DLIST_H
#define LINKEDLISTS_DLIST_H

#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/debug.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dlist.h
 * @brief Defines the interface for doubly linked list.
 */

/**
 * @defgroup DBLYLIST Doubly Linked List
 * @ingroup LINKEDLISTS
 * @brief Basic operations for the generic double linked list list using a sentinel.
 * 
 * @details
 * This is circular linked list but interpreted as doubly linked list. Instead of having two
 * sentinels at both ends, this list has one sentinel by completing a circle.
 * ### Global Constraints
 * - **NULL Pointers**: All `struct dlist *dl` and `struct dlist_item *item` arguments must be non-NULL
 * - **Ownership**: Internal nodes are owned by the list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. dlist_deinit might be helpful to destruct remaining
 * - objects in the list.
 * @{
 */

/**
 * @struct dlist_item
 * @brief nodes that stores void* refs, no need to use this
 * struct generally except allocation.
 * @warning **Null Safety**: All functions taking `struct dlist_item*` 
 * expect a valid, initialized by @ref dlist_item_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct dlist_item {
    struct dlist_item       *prev;      ///< Prev item.
    struct dlist_item       *next;      ///< Next item.
    void                    *data;      ///< Reference to the object.
};

/** @brief Init item. */
void dlist_item_init(struct dlist_item *item, struct dlist_item *prev, struct dlist_item *next, void *data);

/**
 * @struct dlist
 * @brief Generic double linked list.
 */
struct dlist {
    struct dlist_item               sentinel;       ///< Sentinel node.
    size_t                          size;           ///< Count of the objects whose references are stored here.
    struct allocator_concept        ac;             ///< Used by dlist to allocate struct dlist_item to maintain dlist.
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the double linked list.
 * @param[in, out] dl Pointer to the list to be initialized. Must not be NULL or invalid.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes. Must not be NULL or invalid.
 * @see allocator_concept
 */
void dlist_init(struct dlist *dl, struct allocator_concept *ac);

/**
 * @brief Deinits the double linked list.
 * @param[in, out] dl Pointer to the list to be deinitialized. Must not be NULL or invalid.
 * @param[in] oc Pointer to an object_concept used to init/deinit nodes. Must not be NULL or invalid.
 * @warning **Deiniting**: the list given `dl` isnt freed, its fields are.
 * @warning **Object Fields Cleanup**: THe lists fields arent set to zero. Since you might
 * allocate the list on the heap, that would be silly before freeing it.
 * @see object_concept
 */
void dlist_deinit(struct dlist *dl, struct object_concept *oc);

/** @} */ // End of Initialization & Deinitalization

/**
 * @name Insertion
 * Operations to add items to the list.
 * @{
 */

/**
 * @brief Inserts a new item between @p prev_item and @p next_item.
 * They must be in @p dl.
 * @param[in,out] dl Pointer to the list instance.
 * @param[in] prev_item Will be prev item of new item.
 * @param[in] next_item Will be next item of new item.
 * @param[in] new_data Pointer to the new data to insert.
 * @return int, 0 indicates success, non-zero indicates failure
 * @warning **Lifetime Management**: The list does NOT take ownership of the
 * * memory containing @p new_data.
 * @note This operation is **O(1)** (constant time).
 */
int dlist_insert_between(struct dlist *dl, struct dlist_item *prev_item, struct dlist_item *next_item, void *new_data);

/**
 * @brief Inserts an item at the front of the list.
 * @see dlist_insert_before
 */
int dlist_push_front(struct dlist *dl, void *new_data);

/**
 * @brief Inserts an item at the back of the list.
 * @see dlist_insert_after
 */
int dlist_push_back(struct dlist *dl, void *new_data);

/** @} */ // End of Insertion

/**
 * @name Removal
 * Operations to remove items from the list.
 * @{
 */

/**
 * @brief Removes item at specific position.
 * @param[in,out] dl Pointer to the list instance.
 * @param[in] item Iter to be removed from the list.
 * Must be already in a list.
 * @return Data that was stored by `item`, or NULL if list was empty.
 * @warning **Lifetime Management**: The list did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back.
 * @note This operation is **O(1)** (constant time).
 */
void *dlist_remove(struct dlist *dl, struct dlist_item *item);

/**
 * @brief Removes current head item.
 * @return Data that was stored by previous head, or NULL if list was empty.
 * @see dlist_remove
 */
void *dlist_remove_front(struct dlist *dl);

/**
 * @brief Removes current tail item.
 * @return Data that was stored by previous tail, or NULL if list was empty.
 * @see dlist_remove
 */
void *dlist_remove_back(struct dlist *dl);

/** @} */ // End of Removal

/**
 * @name Iteration
 * Operations to iterate items of the list.
 * @{
 */

/** @return prev item. */
static inline struct dlist_item *dlist_item_prev(struct dlist_item *item)
{
    assert(item != NULL);
    return item->prev;
}

/** @return next item. */
static inline struct dlist_item *dlist_item_next(struct dlist_item *item)
{
    assert(item != NULL);
    return item->next;
}

/** @return reference to data kept at item. */
static inline void *dlist_item_data(struct dlist_item *item)
{
    assert(item != NULL);
    return item->data;
}

/**
 * @brief Iterate over items by following prev pointers.
 * @param[in] item Iterator `struct dlist_item *` for loop variable.
 * @param[in] begin Iterator to begin (struct dlist_item *).
 * @param[in] end Iterator to end (struct dlist_item *).
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define dlist_foreach_bk(item, begin, end) \
    for (item = begin; item != end; item = dlist_item_prev(item))

/**
 * @brief Iterate backwards safely by storing next pointer in advance.
 * @param[in] item Loop variable (struct dlist_item *).
 * @param[in] n Temporary storage for prev item (struct dlist_item *).
 * @param[in] begin Iterator to begin at (struct dlist_item *).
 * @param[in] end Iterator to stop at (struct dlist_item *, exclusive).
 */
#define dlist_foreach_bk_safe(item, n, begin, end) \
    for (item = begin, n = dlist_item_prev(item); item != end; item = n, n = dlist_item_prev(n))

/**
 * @brief Iterate over items by following next pointers.
 * @param[in] item Iterator `struct dlist_item *` for loop variable.
 * @param[in] begin Iterator to begin (struct dlist_item *).
 * @param[in] end Iterator to end (struct dlist_item *).
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define dlist_foreach_fr(item, begin, end) \
    for (item = begin; item != end; item = dlist_item_next(item))

/**
 * @brief Iterate forwards safely (allows removal of current node).
 * @param[in] item Loop variable (struct dlist_item *).
 * @param[in] n Temporary storage for next item (struct dlist_item *).
 * @param[in] begin Iterator to begin at (struct dlist_item *).
 * @param[in] end Iterator to stop at (struct dlist_item *, exclusive).
 */
#define dlist_foreach_fr_safe(item, n, begin, end) \
    for (item = begin, n = dlist_item_next(item); item != end; item = n, n = dlist_item_next(n))

/** @} */ // End of Iteration

/**
 * @name Search
 * Operations to itemate items of the list.
 * @{
 */

/**
 * @brief Searches for an item in the double linked list matching a condition, in backwards.
 * Iterates through the list starting from @p begin until @p end and returns the first
 * reference for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * @param result (struct dlist_item *): Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * @param begin Iterator to begin (struct dlist_item *).
 * @param end Iterator to end (struct dlist_item *).
 * @param condition Predicate function or macro that takes a pointer to void*
 *                  reference and returns zero (true) for a match, non-zero (false) otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_bk(result, begin, end, condition)  \
{                                                           \
    result = NULL;                                          \
    struct dlist_item *_item;                               \
    dlist_foreach_bk(_item, begin, end) {                   \
        void *_ref = dlist_item_data(_item);                 \
        if (condition(_ref) == 0) {                         \
            result = _item;                                 \
            break;                                          \
        }                                                   \
    }                                                       \
}

/**
 * @brief Searches for an item in the double linked list matching a condition, in forwards.
 * Iterates through the list starting from @p begin until @p end and returns the first
 * item for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * @param result (struct dlist_item *): Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * @param begin Iterator to begin  (struct dlist_item *).
 * @param end Iterator to end  (struct dlist_item *).
 * @param condition Predicate function or macro that takes a pointer to void*
 *                  reference and returns zero (true) for a match, non-zero (false) otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_fr(result, begin, end, condition)  \
{                                                           \
    result = NULL;                                          \
    struct dlist_item *_item;                               \
    dlist_foreach_fr(_item, begin, end) {                   \
        void *_ref = dlist_item_data(_item);                 \
        if (condition(_ref) == 0) {                         \
            result = _item;                                 \
            break;                                          \
        }                                                   \
    }                                                       \
}

/** @} */ // End of Search

/**
 * @name Inspection
 * Operations to itemate items of the list.
 * @{
 */

/**
 * @brief Checks if the list is empty.
 * @return 1 (true) if empty, 0 (false) otherwise.
 */
int dlist_empty(const struct dlist *dl);

/**
 * @brief Returns the number of items in the list.
 * @return Size of the list.
 */
size_t dlist_size(const struct dlist *dl);

/** @return Iterator to the head of the list. */
struct dlist_item *dlist_head(struct dlist *dl);

/** @return Iterator to the tail of the list. */
struct dlist_item *dlist_tail(struct dlist *dl);

/** @} */ // End of Inspection

/** @brief Reverses */
void dlist_reverse(struct dlist *dl);

/** @} */ // End of DBLYLIST group

#ifdef __cplusplus
}
#endif

#endif // LINKEDLISTS_DLIST_H