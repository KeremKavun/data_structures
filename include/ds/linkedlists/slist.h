#ifndef LINKEDLISTS_SLIST_H
#define LINKEDLISTS_SLIST_H

#include <ds/utils/object_concept.h>
#include <ds/utils/debug.h>
#include <ds/utils/macros.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file slist.h
 * @brief Defines the interface for intrusive singly linked list.
 */

/**
 * @defgroup SINGLYLIST Singly Linked List
 * @ingroup LINKEDLISTS
 * @brief Basic operations for instrusive single linked list list using a sentinel (not circular).
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct slist *sl` and `struct slist_item *iter` arguments must be non-NULL
 * - **Ownership**: Internal nodes are owned by the user since this is intrusive list. slist_deinit might
 * - be helpful to destruct remaining objects in the list.
 * @{
 */

/**
 * @struct slist_item
 * @brief This struct is used to link user data
 * into linked list data structure.
 * @code
 * struct mydata {
 *  int data;
 *  struct slist_item *hook;
 * };
 */
struct slist_item {
    struct slist_item       *next;      ///< Next item.
};

/** @brief Initializes slist_item. */
static inline void slist_item_init(struct slist_item *item, struct slist_item *next)
{
    assert(item != NULL);
    item->next = next;
}

/** @brief Recovers the parent structure pointer from an embedded slist_item. */
#define slist_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @struct slist
 * @brief Generic single linked list.
 */
struct slist {
    struct slist_item               sentinel;       ///< Sentinel node to reduce if checks and NULL safety.
    size_t                          size;           ///< Count of the objects whose references are stored here.
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the single linked list
 * @param[in, out] sl Pointer to the list to be initialized.
 */
void slist_init(struct slist *sl);

/**
 * @brief Deinits the single linked list.
 * @param[in, out] sl Pointer to the list to be deinitialized.
 * @param[in] deinit Pointer to function pointer that receives
 * struct slist_item *pointer and performs deallocation. container_of
 * or slist_entry can be used to recover parent struct inside. Might be
 * NULL, but then no iteration will be done, only sentinel->next is set to NULL
 * and size to 0.
 * @see object_concept
 */
void slist_deinit(struct slist *sl, deinit_cb deinit);

/** @} */ // End of Initialization & Deinitalization

/**
 * @name Insertion & Removal
 * Operations for insertion and removal.
 * @{
 */

/**
 * @brief Inserts a new node at @p pos.
 * @param[in,out] sl Pointer to the list instance.
 * @param[in] pos Pointer to previous nodes next attribute,
 * got by find algorithms at some point.
 * @param[in] new_item Pointer to the new item's hook to insert.
 * @note This operation is **O(1)** (constant time).
 */
void slist_insert(struct slist *sl, struct slist_item **pos, struct slist_item *new_item);

/**
 * @brief Removes item at specific position.
 * @param[in,out] sl Pointer to the list instance.
 * @param[in] item Item to be removed from the list.
 * got by find algorithms at some point.
 * @note If the list is empty, no action is taken.
 * @note This operation is **O(1)** (constant time).
 */
void slist_remove(struct slist *sl, struct slist_item **item);

/** @} */ // End of Insertion & Removal

/**
 * @name Iteration
 * Operations to iterate items of the list.
 * @{
 */

/**
 * @return next item.
 * @note Relies on 'next' being the first field of slist_item.
 */
static inline struct slist_item **slist_item_next(struct slist_item **item)
{
    assert(item != NULL);
    return (struct slist_item **) (*item);
}

/**
 * @brief Iterate over items maximum to the end.
 * @param[in] item: (struct slist **) Iterator for loop variable.
 * @param[in] begin: (struct slist_item **) Iterator to begin.
 * @param end: (struct slist_item **) Iterator to end, NULL to scan all.
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define slist_foreach(item, begin, end) \
    for (item = begin; item != end && *item != NULL; item = slist_item_next(item))

/**
 * @brief Iterate safely (allows removal of current node).
 * @param[in] item: (struct slist_item **) Loop variable.
 * @param[in] n: (struct slist_item **) Temporary storage for next item.
 * @param[in] begin: (struct slist_item **) Iterator to begin at.
 * @param end: (struct slist_item **) Iterator to end, NULL to scan all.
 * @note it does this by storing next pointer, if current item is freed,
 * then remove function should already alter what item points to, which
 * is next pointer, thus we dont call next.
 */
#define slist_foreach_safe(item, n, begin, end)                        \
    for (item = begin;                                                     \
         (item != end && *item != NULL) && ((n = (*item)->next), 1);       \
         item = ((*item) == n ? item : slist_item_next(item)))

/** @} */ // End of Iteration

/**
 * @name Search
 * Operations to itemate items of the list.
 * @{
 */

/**
 * @brief Searches for an item in the single linked list matching a condition.
 * Iterates through the list starting from @p begin until @p end and returns the first
 * item for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * @param result: (typeof(parent struct)) Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * @param begin: (struct slist_item **) Iterator to begin.
 * @param end: (struct slist_item **) Iterator to end, NULL to scan all.
 * @param member Name of parent struct member of type struct slist_item.
 * @param condition Predicate function or macro that takes a pointer to parent struct
 *                  and returns zero for a match, non-zero otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define slist_find_entry(result, begin, end, member, condition)             \
{                                                                           \
    result = NULL;                                                          \
    struct slist_item **_item;                                              \
    slist_foreach(_item, begin, end) {                                      \
        typeof(result) _tmp = slist_entry(*_item, typeof(*result), member); \
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
int slist_empty(const struct slist *sl);

/**
 * @brief Returns the number of items in the list.
 * @return Size of the list.
 */
size_t slist_size(const struct slist *sl);

/** @return Pointer to pointer to the head of the list. */
struct slist_item **slist_head(struct slist *sl);

/** @} */ // End of Inspection

/** @} */ // End of SINGLYLIST group

#ifdef __cplusplus
}
#endif

#endif // LINKEDLISTS_SLIST_H