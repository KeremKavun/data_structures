#ifndef CLIST_H
#define CLIST_H

#include "../../concepts/include/allocator_concept.h"
#include "../../utils/include/macros.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Circular list (intrusive) API
 * * @brief Basic operations for the intrusive list using a Sentinel (Dummy Head).
 * * * ### Global Constraints
 * - **NULL Pointers**: All `struct clist*` and `struct clist_item*` arguments must be non-NULL.
 * - **Ownership**: This is an intrusive list; the user manages all memory.
 * * * ### Sentinel Logic
 * This implementation uses a permanent "sentinel" node embedded in the `clist` struct.
 * - An **Empty** list is a sentinel pointing to itself (next == &sentinel).
 * - **Iterating**: Start at `sentinel.next`. Stop when you reach `&sentinel`.
 * - **No NULLs**: `next` and `prev` pointers are never NULL in a valid list.
 * @{
 */

/* === clist_item === */

/**
 * @struct clist_item
 * @brief An intrusive node hook.
 *
 * Embed this structure inside your own data objects to allow them to be 
 * linked into a @ref clist.
 */
struct clist_item {
    struct clist_item       *prev;
    struct clist_item       *next;
};

/**
 * @brief Initializes the circular linked list item.
 * Sets next and prev to point to itself.
 * * @param[in, out] item Pointer to the item to be initialized. Must not be NULL.
 */
void clist_item_init(struct clist_item *item);

/**
 * @brief Recovers the parent structure pointer from an embedded clist_item.
 */
#define clist_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @struct clist
 * @brief Circular list with Sentinel.
 * * Contains a permanent sentinel node that serves as both the "head" and "tail" 
 * anchor. 
 * * - `sentinel.next` points to the first actual item (Front).
 * - `sentinel.prev` points to the last actual item (Back).
 * * @warning **Null Safety**: All functions expect a valid, initialized pointer.
 */
struct clist {
    struct clist_item       sentinel;
    size_t                  size;
};

/**
 * @name Initialization
 * @{
 */

/**
 * @brief Initializes the list. Sets sentinel to point to itself and size to 0.
 * @param[in, out] cl Pointer to the list. Must not be NULL.
 */
void clist_init(struct clist *cl);

/** @} */

/**
 * @name Insertion
 * @{
 */

/**
 * @brief Inserts a new item immediately before a specific position.
 * @note To push back, insert before the sentinel.
 */
void clist_insert_before(struct clist *cl, struct clist_item *pos, struct clist_item *new_item);

/**
 * @brief Inserts a new item immediately after a specific position.
 * @note To push front, insert after the sentinel.
 */
void clist_insert_after(struct clist *cl, struct clist_item *pos, struct clist_item *new_item);

/**
 * @brief Inserts an item at the front (after sentinel).
 */
void clist_push_front(struct clist *cl, struct clist_item *new_item);

/**
 * @brief Inserts an item at the back (before sentinel).
 */
void clist_push_back(struct clist *cl, struct clist_item *new_item);

/** @} */

/**
 * @name Removal
 * @{
 */

/**
 * @brief Unlinks an item from the list.
 * @note **O(1)**.
 */
void clist_remove(struct clist *cl, struct clist_item *item);

/**
 * @brief Removes the first item (sentinel.next).
 * @return Pointer to the removed item, or NULL if list was empty.
 */
struct clist_item *clist_pop_front(struct clist *cl);

/**
 * @brief Removes the last item (sentinel.prev).
 * @return Pointer to the removed item, or NULL if list was empty.
 */
struct clist_item *clist_pop_back(struct clist *cl);

/** @} */

/**
 * @name Iteration
 * @{
 */

/**
 * @return prev item.
 */
static inline struct clist_item *clist_item_prev(struct clist_item *item)
{
    return item->prev;
}

/**
 * @return next item.
 */
static inline struct clist_item *clist_item_next(struct clist_item *item)
{
    return item->next;
}

/**
 * @brief Iterates over the list clock-wise (forward).
 * @param pos Loop variable (`struct clist_item*`).
 * @param head Pointer to the list (`struct clist*`).
 */
#define clist_foreach_cw(pos, head) \
    for (pos = (head)->sentinel.next; pos != &(head)->sentinel; pos = pos->next)

/**
 * @brief Iterates over the list counter-clock-wise (backward).
 * @param pos Loop variable (`struct clist_item*`).
 * @param head Pointer to the list (`struct clist*`).
 */
#define clist_foreach_ccw(pos, head) \
    for (pos = (head)->sentinel.prev; pos != &(head)->sentinel; pos = pos->prev)

/**
 * @brief Iterates over parent structures safe against removal.
 * * @param obj Iterator variable for parent structure pointer.
 * @param head Pointer to the list (`struct clist*`).
 * @param member Name of the `struct clist_item` member within parent.
 */
#define clist_foreach_entry(obj, head, member) \
    for (struct clist_item *_iter = (head)->sentinel.next; \
         _iter != &(head)->sentinel && (obj = clist_entry(_iter, typeof(*obj), member)); \
         _iter = _iter->next)

/** @} */

/**
 * @name Search
 * @{
 */

/**
 * @brief Searches for an item matching a condition (Counter-Clockwise).
 * @param result Variable to store result (NULL if not found).
 * @param head Pointer to the list (`struct clist*`).
 * @param member Name of struct member.
 * @param condition Macro/Function returning true on match.
 */
#define clist_find_entry_bk(result, head, member, condition) do { \
    result = NULL; \
    struct clist_item *_p; \
    clist_foreach_ccw(_p, head) { \
        typeof(result) _tmp = clist_entry(_p, typeof(*result), member); \
        if (condition(_tmp)) { \
            result = _tmp; \
            break; \
        } \
    } \
} while(0)

/**
 * @brief Searches for an item matching a condition (Clockwise).
 */
#define clist_find_entry_fr(result, head, member, condition) do { \
    result = NULL; \
    struct clist_item *_p; \
    clist_foreach_cw(_p, head) { \
        typeof(result) _tmp = clist_entry(_p, typeof(*result), member); \
        if (condition(_tmp)) { \
            result = _tmp; \
            break; \
        } \
    } \
} while(0)

/** @} */

/**
 * @name Inspection
 * @{
 */

/**
 * @brief Returns the sentinel item.
 */
static inline struct clist_item *get_clist_sentinel(struct clist *cl) {
    return &cl->sentinel;
}

/**
 * @brief Checks if the list is empty.
 */
static inline int clist_empty(const struct clist *cl) {
    return (cl->sentinel.next == &cl->sentinel);
}

/**
 * @brief Returns the number of items.
 */
static inline size_t clist_size(const struct clist *cl) {
    return cl->size;
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // CLIST_H