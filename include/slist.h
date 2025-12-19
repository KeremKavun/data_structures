#ifndef SLIST_H
#define SLIST_H

#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Single linked list API
 * 
 * @brief Basic operations for the generic single linked list list.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct slist *sl` and `struct slist_item *iter` arguments must be non-NULL
 * - **Ownership**: Internal nodes are owned by the list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. slist_free might be helpful to destruct remaining
 * - objects in the list.
 * @{
 */

/**
 * @struct slist_item
 * 
 * @brief nodes that stores void* refs, no need to use this
 * struct generally except allocation.
 * 
 * @warning **Null Safety**: All functions taking `struct slist_item*` (including *(struct slist_item **))
 * expect a valid, initialized by @ref slist_item_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct slist_item {
    struct slist_item       *next;
    void                    *data;
};

/**
 * @brief Init item.
 */
void slist_item_init(struct slist_item *item, struct slist_item *next, void *data);

/**
 * @struct slist
 * 
 * @brief Generic single linked list.
 * 
 * @warning **Null Safety**: All functions taking `struct slist*` 
 * expect a valid, initialized by @ref slist_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct slist {
    struct slist_item               sentinel;
    size_t                          size;
    struct allocator_concept*       ac;
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the single linked list
 * 
 * @param[in, out] sl Pointer to the list to be initialized. Must not be NULL or invalid.
 * @param[in] ac Pointer to an allocator_concept used to allocate nodes. Must not be NULL or invalid.
 * 
 * @see allocator_concept
 */
void slist_init(struct slist *sl, struct allocator_concept *ac);

/**
 * @brief Deinits the single linked list.
 * 
 * @param[in, out] sl Pointer to the list to be deinitialized. Must not be NULL or invalid.
 * @param[in] context Pointer to a arbitrary context for ease. Will be passed to the deinit method's void *context parameter.
 * @param[in] oc Pointer to an object_concept used to init/deinit nodes. Must not be NULL or invalid.
 * 
 * @warning **Deiniting**: the list given `sl` isnt freed, its fields are.
 * @warning **Object Fields Cleanup**: The lists fields arent set to zero. Since you might
 * allocate the list on the heap, that would be silly before freeing it.
 * 
 * @see object_concept
 */
void slist_deinit(struct slist *sl, void *context, struct object_concept *oc);

/** @} */ // End of Initialization & Deinitalization group

/**
 * @name Insertion
 * Operations to add items to the list.
 * @{
 */

/**
 * @brief Inserts a new node at @p pos.
 * 
 * @param[in,out] sl Pointer to the list instance.
 * @param[in] pos Pointer to previous nodes next attribute,
 * got by find algorithms at some point.
 * @param[in] new_data Pointer to the new data to insert.
 * 
 * @return int, 0 indicates success, non-zero indicates failure
 * 
 * @warning **Lifetime Management**: The list does NOT take ownership of the
 * * memory containing @p new_data.
 * 
 * @note This operation is **O(1)** (constant time).
 */
int slist_insert(struct slist *sl, struct slist_item **pos, void *new_data);

/** @} */ // End of Insertion group

/**
 * @name Removal
 * Operations to remove items from the list.
 * @{
 */

/**
 * @brief Removes item at specific position.
 * 
 * @param[in,out] sl Pointer to the list instance.
 * @param[in] item Item to be removed from the list.
 * got by find algorithms at some point.
 * 
 * @return Data that was stored by `item`, or NULL if list was empty.
 * 
 * @warning **Lifetime Management**: The list did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back
 * 
 * @note This operation is **O(1)** (constant time).
 */
void *slist_remove(struct slist *sl, struct slist_item **item);

/** @} */ // End of Removal group

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
 * @return reference to data kept at item.
 */
static inline void *slist_item_data(struct slist_item **item)
{
    assert(item != NULL);
    return (*item)->data;
}

/**
 * @brief Iterate over items maximum to the end.
 * 
 * @param[in] sl Pointer to the list instance.
 * @param[in] item Iterator `struct slist_item **` for loop variable.
 * @param[in] begin Iterator to begin (struct slist_item **).
 * @param[in] end Iterator to end (struct slist_item **).
 * 
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define slist_foreach(sl, item, begin, end) \
    for (item = begin; item != end && *item != NULL; item = slist_item_next(item))

/**
 * @brief Iterate safely (allows removal of current node).
 * 
 * @param[in] sl Pointer to the list instance.
 * @param[in] item Loop variable (struct slist_item **).
 * @param[in] n Temporary storage for next item (struct slist_item **).
 * @param[in] begin Iterator to begin at (struct slist_item **).
 * 
 * @param[in] end Iterator to stop at (struct slist_item **, exclusive).
 * 
 * @note it does this by storing next pointer, if current item is freed,
 * then remove function should already alter what item points to, which
 * is next pointer, thus we dont call next.
 */
#define slist_foreach_safe(sl, item, n, begin, end)                                 \
    for (item = begin;                                                              \
         (item != end && *item != NULL) && ((n = (*item)->next), 1);       \
         item = ((*item) == n ? item : slist_item_next(item)))

/** @} */ // End of Iteration group

/**
 * @name Search
 * Operations to itemate items of the list.
 * @{
 */

/**
 * @brief Searches for an item in the single linked list matching a condition.
 * 
 * Iterates through the list starting from @p begin until @p end and returns the first
 * item for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * 
 * @param result (struct slist_item **): Variable to store the result.
 *               Will be set to NULL if no match is found, or to the matching
 *               reference if found. Should be pre-declared.
 * 
 * @param begin Iterator to begin  (struct slist_item **).
 * @param end Iterator to end  (struct slist_item **).
 * @param condition Predicate function or macro that takes a pointer to void*
 *                  reference and returns zero (true) for a match, non-zero (false) otherwise.
 * 
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define slist_find_entry(result, sl, begin, end, condition) \
{                                                           \
    result = NULL;                                          \
    struct slist_item **_item;                              \
    slist_foreach(sl, _item, begin, end) {                  \
        void *_ref = slist_item_data(_item);                \
        if (condition(_ref) == 0) {                         \
            result = _item;                                 \
            break;                                          \
        }                                                   \
    }                                                       \
}

/** @} */ // End of Search group

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

/**
 * @return Pointer to pointer to the head of the list.
 */
struct slist_item **slist_head(struct slist *sl);

/** @} */ // End of Inspection group

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // SLIST_H