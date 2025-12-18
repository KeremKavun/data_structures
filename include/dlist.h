#ifndef DLIST_H
#define DLIST_H

#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Double linked list API
 * @brief Basic operations for the generic double linked list list.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct dlist *dl` and `dlist_iter_t iter` arguments must be non-NULL
 * - **Ownership**: Internal nodes are owned by the list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. dlist_free might be helpful to destruct remaining
 * - objects in the list.
 * @{
 */

/**
 * @brief Double linked list node iterator. Must not be NULL whenever used in functions uses them.
 */
typedef struct dlist_item* dlist_iter_t;

/**
 * @struct dlist
 * @brief Generic double linked list.
 * @warning **Null Safety**: All functions taking `struct dlist*` 
 * expect a valid, initialized by @ref dlist_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct dlist {
    struct dlist_item               sentinel;
    size_t                          size;
    struct allocator_concept        *ac;
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the double linked list
 * 
 * @param[in, out] dl Pointer to the list to be initialized. Must not be NULL or invalid.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes. Must not be NULL or invalid.
 * 
 * @see allocator_concept
 */
void dlist_init(struct dlist *dl, struct allocator_concept *ac);

/**
 * @brief Deinits the double linked list.
 * 
 * @param[in, out] dl Pointer to the list to be deinitialized. Must not be NULL or invalid.
 * @param[in] context Pointer to a arbitrary context for ease. Will be passed to the deinit method's void *context parameter.
 * @param[in] oc Pointer to an object_concept used to init/deinit nodes. Must not be NULL or invalid.
 * 
 * @warning **Deiniting**: the list given `dl` isnt freed, its fields are.
 * @warning **Object Fields Cleanup**: THe lists fields arent set to zero. Since you might
 * allocate the list on the heap, that would be silly before freeing it.
 * 
 * @see object_concept
 */
void dlist_deinit(struct dlist *dl, void *context, struct object_concept *oc);

/** @} */ // End of Initialization group

/**
 * @name Insertion
 * Operations to add items to the list.
 * @{
 */

/**
 * @brief Inserts a new item between @p prev_iter and @p next_iter.
 * They must be in @p dl.
 * 
 * @param[in,out] dl Pointer to the list instance.
 * @param[in] prev_iter Will be prev iter of new item.
 * @param[in] next_iter Will be next iter of new item.
 * @param[in] new_data Pointer to the new data to insert.
 * 
 * @return int, 0 indicates success, non-zero indicates failure
 * 
 * @warning **Lifetime Management**: The list does NOT take ownership of the
 * * memory containing @p new_data.
 * 
 * @note This operation is **O(1)** (constant time).
 */
int dlist_insert_between(struct dlist *dl, dlist_iter_t prev_iter, dlist_iter_t next_iter, void *new_data);

/**
 * @brief Inserts an item at the front of the list.
 * 
 * @see dlist_insert_before
 */
int dlist_push_front(struct dlist *dl, void *new_data);

/**
 * @brief Inserts an item at the back of the list.
 * 
 * @see dlist_insert_after
 */
int dlist_push_back(struct dlist *dl, void *new_data);

/** @} */ // End of Insertion group

/**
 * @name Removal
 * Operations to remove items from the list.
 * @{
 */

/**
 * @brief Removes item at specific position.
 * 
 * @param[in,out] dl Pointer to the list instance.
 * @param[in] iter Iter to be removed from the list.
 * Must be already in a list.
 * 
 * @return Data that was stored by `iter`, or NULL if list was empty.
 * 
 * @warning **Lifetime Management**: The list did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back
 * 
 * @note This operation is **O(1)** (constant time).
 */
void *dlist_remove(struct dlist *dl, dlist_iter_t *iter);

/**
 * @brief Removes current head item.
 * @return Data that was stored by previous head, or NULL if list was empty.
 * 
 * @see dlist_remove
 */
void *dlist_remove_front(struct dlist *dl);

/**
 * @brief Removes current tail item.
 * @return Data that was stored by previous tail, or NULL if list was empty.
 * 
 * @see dlist_remove
 */
void *dlist_remove_back(struct dlist *dl);

/** @} */ // End of Removal group

/**
 * @name Iteration
 * Operations to iterate items of the list.
 * @{
 */

/**
 * @return prev iterator.
 */
dlist_iter_t dlist_prev(dlist_iter_t iter);

/**
 * @return next iterator.
 */
dlist_iter_t dlist_next(dlist_iter_t iter);

/**
 * @return reference to data kept at iterator.
 */
void *dlist_get_data(dlist_iter_t iter);

/**
 * @brief Iterate over references to the data user provided following prev iterators
 * @param[in] iter Iterator `dlist_iter_t` for loop variable
 * @param[in] begin Iterator to begin (dlist_iter_t).
 * @param[in] end Iterator to end (dlist_iter_t).
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define dlist_foreach_bk(iter, begin, end) \
    for (iter = begin; iter != end; iter = dlist_prev(iter))

/**
 * @brief Iterate backwards safely by storing next pointer in advance.
 * @param[in] curr Loop variable (dlist_iter_t).
 * @param[in] n Temporary storage for next iterator (dlist_iter_t).
 * @param[in] begin Iterator to begin at (dlist_iter_t).
 * @param[in] end Iterator to stop at (dlist_iter_t, exclusive).
 */
#define dlist_foreach_fr_safe(curr, n, begin, end) \
    for (curr = begin, n = dlist_next(curr); curr != end; curr = n, n = dlist_next(n))

/**
 * @brief Iterate over references to the data user provided following next iterators
 * @param[in] iter Iterator `dlist_iter_t` for loop variable
 * @param[in] begin Iterator to begin (dlist_iter_t).
 * @param[in] end Iterator to end (dlist_iter_t).
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define dlist_foreach_fr(iter, begin, end) \
    for (iter = begin; iter != end; iter = dlist_next(iter))

/**
 * @brief Iterate forwards safely (allows removal of current node).
 * @param[in] curr Loop variable (dlist_iter_t).
 * @param[in] n Temporary storage for next iterator (dlist_iter_t).
 * @param[in] begin Iterator to begin at (dlist_iter_t).
 * @param[in] end Iterator to stop at (dlist_iter_t, exclusive).
 */
#define dlist_foreach_fr_safe(curr, n, begin, end) \
    for (curr = begin, n = dlist_next(curr); curr != end; curr = n, n = dlist_next(n))

 /** @} */ // End of Iteration group

 /**
 * @name Search
 * Operations to iterate items of the list.
 * @{
 */

/**
 * @brief Searches for an item in the double linked list matching a condition, in backwards.
 * 
 * Iterates through the list starting from @p begin until @p end and returns the first
 * reference for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * 
 * @param result Variable to store the result (void*).
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * 
 * @param begin Iterator to begin (dlist_iter_t).
 * @param end Iterator to end (dlist_iter_t).
 * @param condition Predicate function or macro that takes a pointer to void*
 *                  reference and returns zero (true) for a match, non-zero (false) otherwise.
 * 
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_bk(result, begin, end, condition)  \
{                                                           \
    result = NULL;                                          \
    dlist_iter_t _iter;                                     \
    dlist_foreach_bk(_iter, begin, end) {                   \
        void *_ref = dlist_get_data(_iter);                 \
        if (condition(_ref) == 0) {                         \
            result = _ref;                                  \
            break;                                          \
        }                                                   \
    }                                                       \
}

/**
 * @brief Searches for an item in the double linked list matching a condition, in forwards.
 * 
 * Iterates through the list starting from @p begin until @p end and returns the first
 * reference for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * 
 * @param result Variable to store the result (void*).
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * 
 * @param begin Iterator to begin  (dlist_iter_t).
 * @param end Iterator to end  (dlist_iter_t).
 * @param condition Predicate function or macro that takes a pointer to void*
 *                  reference and returns zero (true) for a match, non-zero (false) otherwise.
 * 
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define dlist_find_entry_fr(result, begin, end, condition)  \
{                                                           \
    result = NULL;                                          \
    dlist_iter_t _iter;                                     \
    dlist_foreach_fr(_iter, begin, end) {                   \
        void *_ref = dlist_get_data(_iter);                 \
        if (condition(_ref) == 0) {                         \
            result = _ref;                                  \
            break;                                          \
        }                                                   \
    }                                                       \
}

 /** @} */ // End of Search group

 /**
 * @name Inspection
 * Operations to iterate items of the list.
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

/**
 * @return Iterator to the head of the list.
 */
dlist_iter_t dlist_head(struct dlist *dl);

/**
 * @return Iterator to the tail of the list.
 */
dlist_iter_t dlist_tail(struct dlist *dl);

 /** @} */ // End of Inspection group

void dlist_reverse(struct dlist *dl);

 /** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // DLIST_H