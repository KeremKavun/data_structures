#ifndef LINKEDLISTS_DLIST_H
#define LINKEDLISTS_DLIST_H

#include <ds/utils/object_concept.h>
#include <ds/utils/debug.h>
#include <ds/utils/macros.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dlist.h
 * @brief Defines the interface for intrusive doubly linked list.
 */

/**
 * @defgroup DBLYLIST Doubly Linked List
 * @ingroup LINKEDLISTS
 * @brief Basic operations for intrusive double linked list list using a sentinel.
 * 
 * @details
 * This is circular linked list but interpreted as doubly linked list. Instead of having two
 * sentinels at both ends, this list has one sentinel by completing a circle.
 * ### Global Constraints
 * - **NULL Pointers**: All `struct dlist *dl` and `struct dlist_item *item` arguments must be non-NULL
 * - **Ownership**: Internal nodes are owned by the user since this is intrusive list.
 * - dlist_deinit might be helpful to destruct remaining objects in the list.
 * @{
 */

/**
 * @struct dlist_item
 * @brief This struct is used to link user data
 * into linked list data structure.
 * @code
 * struct mydata {
 *  int data;
 *  struct dlist_item *hook;
 * };
 */
struct dlist_item {
    struct dlist_item       *prev;      ///< Prev item.
    struct dlist_item       *next;      ///< Next item.
};

/** @brief Initializes dlist_item. */
void dlist_item_init(struct dlist_item *item, struct dlist_item *prev, struct dlist_item *next);

/** @brief Recovers the parent structure pointer from an embedded dlist_item. */
#define dlist_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @struct dlist
 * @brief Generic double linked list.
 */
struct dlist {
    struct dlist_item               sentinel;       ///< Sentinel node.
    size_t                          size;           ///< Count of the objects whose references are stored here.
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the double linked list.
 * @param[in, out] dl Pointer to the list to be initialized.
 */
void dlist_init(struct dlist *dl);

/**
 * @brief Deinits the double linked list.
 * @param[in, out] dl Pointer to the list to be deinitialized.
 * @param[in] deinit Pointer to function pointer that receives
 * struct slist_item *pointer and performs deallocation. container_of
 * or slist_entry can be used to recover parent struct inside. Might be
 * NULL, but then no iteration will be done, only sentinel->next is set to sentinel
 * and size to 0.
 * @see object_concept
 */
void dlist_deinit(struct dlist *dl, deinit_cb deinit);

/** @} */ // End of Initialization & Deinitalization

/**
 * @name Insertion
 * Operations to add items to the list.
 * @{
 */

/**
 * @brief Inserts a new item after the specified item.
 * @param[in] pos The item after which the new item will be inserted.
 * @param[in] new_item The new item to insert.
 */
void dlist_insert_after(struct dlist *dl, struct dlist_item *pos, struct dlist_item *new_item);

/**
 * @brief Inserts a new item before the specified item.
 * @param[in] pos The item before which the new item will be inserted.
 * @param[in] new_item The new item to insert.
 */
void dlist_insert_before(struct dlist *dl, struct dlist_item *pos, struct dlist_item *new_item);

/**
 * @brief Inserts an item at the front of the list.
 * @see dlist_insert_before
 */
void dlist_push_front(struct dlist *dl, struct dlist_item *new_item);

/**
 * @brief Inserts an item at the back of the list.
 * @see dlist_insert_after
 */
void dlist_push_back(struct dlist *dl, struct dlist_item *new_item);

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
 * @return The parameter `item` or NULL if the list is empty
 * (The user passed &dl->sentinel after obtaining a pointer
 * and iterating until the sentinel).
 * @note Fields of item are set to NULL.
 * @note If the list is empty, no action is taken.
 * @note This operation is **O(1)** (constant time).
 */
struct dlist_item *dlist_remove(struct dlist *dl, struct dlist_item *item);

/**
 * @brief Removes current head item.
 * @see dlist_remove
 */
struct dlist_item *dlist_remove_front(struct dlist *dl);

/**
 * @brief Removes current tail item.
 * @return Data that was stored by previous tail, or NULL if list was empty.
 * @see dlist_remove
 */
struct dlist_item *dlist_remove_back(struct dlist *dl);

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

/**
 * @brief Iterate over items by following prev pointers.
 * @param[in] item: (struct dlist_item *) Iterator for loop variable.
 * @param[in] begin: (struct dlist_item *) Iterator to begin.
 * @param[in] end: (struct dlist_item *) Iterator to end.
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define dlist_foreach_bk(item, begin, end) \
    for (item = begin; item != end; item = dlist_item_prev(item))

/**
 * @brief Iterate backwards safely by storing next pointer in advance.
 * @param[in] item: (struct dlist_item *) Loop variable.
 * @param[in] n: (struct dlist_item *) Temporary storage for prev item.
 * @param[in] begin: (struct dlist_item *) Iterator to begin at.
 * @param[in] end: (struct dlist_item *) Iterator to stop at (exclusive).
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
 * @param result: (typeof(parent struct)) Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * @param begin: (struct dlist_item *) Iterator to begin.
 * @param end: (struct dlist_item *) Iterator to end.
 * @param member Name of parent struct member of type struct dlist_item.
 * @param condition Predicate function or macro that takes a pointer to parent struct
 *                  and returns zero for a match, non-zero otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_bk(result, begin, end, member, condition)          \
{                                                                           \
    result = NULL;                                                          \
    struct dlist_item *_item;                                               \
    dlist_foreach_bk(_item, begin, end) {                                   \
        typeof(result) _tmp = dlist_entry(_item, typeof(*result), member);  \
        if (condition(_tmp) == 0) {                                         \
            result = _tmp;                                                  \
            break;                                                          \
        }                                                                   \
    }                                                                       \
}

/**
 * @brief Searches for an item in the double linked list matching a condition, in forwards.
 * Iterates through the list starting from @p begin until @p end and returns the first
 * item for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * @param result: (typeof(parent struct)) Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * @param begin: (struct dlist_item *) Iterator to begin.
 * @param end: (struct dlist_item *) Iterator to end.
 * @param member Name of parent struct member of type struct dlist_item.
 * @param condition Predicate function or macro that takes a pointer to parent struct
 *                  and returns zero for a match, non-zero otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_fr(result, begin, end, member, condition)          \
{                                                                           \
    result = NULL;                                                          \
    struct dlist_item *_item;                                               \
    dlist_foreach_fr(_item, begin, end) {                                   \
        typeof(result) _tmp = dlist_entry(_item, typeof(*result), member);  \
        if (condition(_tmp) == 0) {                                         \
            result = _tmp;                                                  \
            break;                                                          \
        }                                                                   \
    }                                                                       \
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
static inline int dlist_empty(const struct dlist *dl)
{
    assert(dl != NULL);
    return (dl->size == 0);
}

/**
 * @brief Returns the number of items in the list.
 * @return Size of the list.
 */
static inline size_t dlist_size(const struct dlist *dl)
{
    assert(dl != NULL);
    return dl->size;
}

/** @return Iterator to the head of the list. */
static inline struct dlist_item *dlist_head(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.next;
}

/** @return Iterator to the tail of the list. */
static inline struct dlist_item *dlist_tail(struct dlist *dl)
{
    assert(dl != NULL);
    return dl->sentinel.prev;
}

/** @} */ // End of Inspection

/** @brief Reverses */
void dlist_reverse(struct dlist *dl);

/** @} */ // End of DBLYLIST group

#ifdef __cplusplus
}
#endif

#endif // LINKEDLISTS_DLIST_H