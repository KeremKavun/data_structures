#ifndef CLIST_H
#define CLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"
#include "../../utils/include/macros.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

/**
 * @defgroup Circular list (intrusive) API
 * @brief Basic operations for the intrusive list.
 * * ### Global Constraints
 * - **NULL Pointers**: All `cl` and `item` arguments must be non-NULL
 * - **Ownership**: This is an intrusive list; the user manages all memory.
 * @{
 */

/* === clist_item === */

/**
 * @struct clist_item
 * @brief An intrusive node hook.
 *
 * Embed this structure inside your own data objects to allow them to be 
 * linked into a @ref clist.
 * * @code
 * struct my_data
 * {
 *   int id;
 *   struct clist_item hook;
 * };
 * @endcode
 * * Use the macro @ref clist_entry to recover the parent pointer.
 * 
 * @warning **Null Safety**: All functions taking `struct clist_item*` 
 * expect a valid, initialized by @ref clist_item_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct clist_item
{
    struct clist_item* prev;
    struct clist_item* next;
};

/**
 * @brief Intializes the circular linked list item. Use this function to initialize
 * hook to the list while initializing your object.
 * 
 * @param[in, out] item Pointer to the item to be initialized. Must not be NULL.
 */
void clist_item_init(struct clist_item* item);

/**
 * @brief Recovers the parent structure pointer from an embedded clist_item.
 * @param ptr Pointer to the clist_item member.
 * @param type Type of the parent structure.
 * @param member Name of the clist_item member within the parent.
 */
#define clist_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @struct clist
 * @brief Circular list with cursor semantics.
 * 
 * The cursor represents the "front" of the list. All front operations
 * (push_front, pop_front) may modify the cursor position. Back operations
 * (push_back, pop_back) operate relative to the cursor but never move it.
 * 
 * Position-based operations (insert_before, insert_after, remove) never
 * modify the cursor except when removing the cursor itself (moves to next).
 * 
 * Mental model:
 *   cursor = front
 *   cursor->prev = back
 *   The cursor is your "view window" into the circular structure.
 * @warning **Null Safety**: All functions taking `struct clist*` 
 * expect a valid, initialized by @ref clist_init, non-NULL pointer. Behavior is undefined otherwise.
 */
struct clist
{
    struct clist_item* cursor;
    size_t size;
};

/**
 * @name Initialization
 * Functions for setting up the list.
 * @{
 */

/**
 * @brief Initializes the circular linked list
 * @param[in, out] cl Pointer to the list to be initialized. Must not be NULL.
 */
void clist_init(struct clist* cl);

/** @} */ // End of Initialization group

/**
 * @name Insertion
 * Operations to add items to the list.
 * @{
 */

/**
 * @brief Inserts a new item immediately before a specific position. The `struct clist_item* pos`
 * must be currently inside @p cl. Does not change the cursor.
 * 
 * @param[in,out] cl Pointer to the list instance.
 * @param[in] pos Pointer to the item in the list to insert before.
 * @param[in] new_item Pointer to the new item to insert. Must not already be in a list.
 * 
 * @warning **Lifetime Management**: The list does NOT take ownership of the
 * * memory containing @p new_item. If you free the object containing this
 * * item without removing it from the list first, the list will become corrupt.
 * 
 * @note This operation is **O(1)** (constant time).
 * 
 * @see clist_remove()
 * @see clist_push_front()
 */
void clist_insert_before(struct clist* cl, struct clist_item* pos, struct clist_item* new_item);

/**
 * @brief Inserts a new item immediately after a specific position. The`struct clist_item* pos`
 * must be currently inside @p cl. Does not change the cursor.
 * 
 * @param[in,out] cl Pointer to the list instance.
 * @param[in] pos Pointer to the item in the list to insert after.
 * @param[in] new_item Pointer to the new item to insert.
 * Must not be NULL and must not already be in a list.
 * 
 * @warning **Lifetime Management**: The list does NOT take ownership of the 
 * memory containing @p new_item. If you free the object containing this 
 * item without removing it from the list first, the list will become corrupt.
 * 
 * @note This operation is **O(1)** (constant time).
 * 
 * @see clist_remove()
 * @see clist_push_back()
 */
void clist_insert_after(struct clist* cl, struct clist_item* pos, struct clist_item* new_item);

/**
 * @brief Inserts an item at the front of the list and changes `cursor` to `new_item`
 * 
 * @see clist_insert_before
 */
void clist_push_front(struct clist* cl, struct clist_item* new_item);

/**
 * @brief Inserts an item at the back of the list.
 * 
 * @see clist_insert_after
 */
void clist_push_back(struct clist* cl, struct clist_item* new_item);

/** @} */ // End of Insertion group

/**
 * @name Removal
 * Operations to remove items from the list.
 * @{
 */

/**
 * @brief Removes item at specific position. The `struct clist_item* item` must be
 * currently inside @p cl. Moves cursor to the next if item to be deleted is cursor.
 * 
 * @param[in,out] cl Pointer to the list instance.
 * @param[in] item Pointer to the item in the list to be removed. Must not be NULL and must be already be in a list.
 * 
 * @warning **Lifetime Management**: The list did NOT take ownership of the 
 * memory containing @p item. Removing it does not free that memory.
 * 
 * @warning **struct clist_item* item members**: The items members arent set
 * to NULL or itself but users already requested to initialize the item to use this api.
 * 
 * @note This operation is **O(1)** (constant time).
 */
void clist_remove(struct clist* cl, struct clist_item* item);

/**
 * @brief Removes cursor and moves cursor one next.
 * @return Pointer to the (previous) cursor, or NULL if list was empty.
 * 
 * @see clist_remove
 */
struct clist_item* clist_pop_front(struct clist* cl);

/**
 * @brief Removes an item at the back (one prev of the cursor) of the list.
 * @return Pointer to the back, or NULL if list was empty.
 * 
 * @see clist_remove
 */
struct clist_item* clist_pop_back(struct clist* cl);

/** @} */ // End of Removal group

/**
 * @name Iteration
 * Operations to iterate items of the list.
 * @{
 */

/**
 * @brief Iterates over a circular list clock-wise (following next pointers).
 * @param pos Pointer to a `struct clist_item*` to use as the loop variable.
 * @param cursor The starting point (typically `cl->cursor`). Must not be NULL.
 * @note The list must not be modified during iteration (no insertions/removals).
 * @note Use this when you need to iterate in insertion order (front to back).
 */
#define clist_foreach_cw(pos, cursor) \
    for (pos = (cursor); pos != NULL; pos = (pos->next == (cursor) ? NULL : pos->next))

/**
 * @brief Iterates over a circular list counter-clock-wise (following prev pointers).
 * @param pos Pointer to a `struct clist_item*` to use as the loop variable.
 * @param cursor The starting point (typically `cl->cursor`). Must not be NULL.
 * @note The list must not be modified during iteration (no insertions/removals).
 * @note Use this when you need to iterate in reverse order (back to front).
 */
#define clist_foreach_ccw(pos, cursor) \
    for (pos = (cursor); pos != NULL; pos = (pos->prev == (cursor) ? NULL : pos->prev))

/**
 * @brief Iterates over parent structures containing circular list items.
 * 
 * A convenience macro that iterates through the list and automatically extracts
 * the parent structure pointer for each item using @ref clist_entry. This is
 * the preferred way to iterate when you need to access the data structures
 * rather than the raw list items.
 * 
 * @param obj Variable name for the parent structure pointer (will be declared
 *            and assigned automatically). Should be uninitialized.
 * @param head The starting point (`struct clist_item*`, typically `cl->cursor`).
 *             Must not be NULL.
 * @param member Name of the `struct clist_item` member within the parent structure.
 * 
 * @warning **IMPLEMENTATION BUG**: This macro has a double-assignment bug and
 *          may skip the first iteration. Use with caution or prefer manual
 *          iteration with `clist_foreach_cw` + `clist_entry`.
 * 
 * @note The list must not be modified during iteration (no insertions/removals).
 */
#define clist_foreach_entry(obj, head, member)                                \
    for (struct clist_item* _curr = (head);                                   \
         _curr != NULL && (obj = clist_entry(_curr, typeof(*obj), member), 1);\
         _curr = (_curr->next == (head) ? NULL : _curr->next))

/** @} */ // End of Iteration group

/**
 * @name Search
 * Operations to search for items in the list.
 * @{
 */

/**
 * @brief Searches for an item in the circular list matching a condition, in counter clock-wise direction.
 * 
 * Iterates through the list starting from @p head and returns the first parent
 * structure for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * 
 * @param result Variable to store the result (pointer to parent structure).
 *               Will be set to NULL if no match is found, or to the matching
 *               structure pointer if found. Should be pre-declared.
 * @param head The starting point (`struct clist_item*`, typically `cl->cursor`). Must not be NULL.
 * @param member Name of the `struct clist_item` member within the parent structure.
 * @param condition Predicate function or macro that takes a pointer to the parent
 *                  structure and returns zero (true) for a match, non-zero (false) otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define clist_find_entry_bk(result, head, member, condition) { \
    result = NULL;                                          \
    struct clist_item *_p;                                  \
    clist_foreach_ccw(_p, head) {                               \
        typeof(result) _tmp = clist_entry(_p, typeof(*result), member); \
        if (condition(_tmp)) {                              \
            result = _tmp;                                  \
            break;                                          \
        }                                                   \
    }                                                       \
}

/**
 * @brief Searches for an item in the circular list matching a condition, in clock-wise direction.
 * 
 * Iterates through the list starting from @p head and returns the first parent
 * structure for which the @p condition predicate returns true (non-zero).
 * If no match is found, @p result is set to NULL.
 * 
 * @param result Variable to store the result (pointer to parent structure).
 *               Will be set to NULL if no match is found, or to the matching
 *               structure pointer if found. Should be pre-declared.
 * @param head The starting point (`struct clist_item*`, typically `cl->cursor`).
 *             Must not be NULL.
 * @param member Name of the `struct clist_item` member within the parent structure.
 * @param condition Predicate function or macro that takes a pointer to the parent
 *                  structure and returns zero (true) for a match, non-zero (false)
 *                  otherwise.
 * @note This performs a linear search - **O(n)** time complexity.
 * @note Search stops at the first match (does not find all matches).
 */
#define clist_find_entry_fr(result, head, member, condition) { \
    result = NULL;                                          \
    struct clist_item *_p;                                  \
    clist_foreach_cw(_p, head) {                               \
        typeof(result) _tmp = clist_entry(_p, typeof(*result), member); \
        if (condition(_tmp)) {                              \
            result = _tmp;                                  \
            break;                                          \
        }                                                   \
    }                                                       \
}

/** @} */ // End of Search group

/**
 * @name Inspection
 * Functions to query list state or retrieve bounds.
 * @{
 */

/**
 * @brief Returns the cursor.
 * @return Pointer to cursor item, or NULL if empty.
 */
struct clist_item* get_clist_cursor(struct clist* cl);

/**
 * @brief Checks if the list is empty.
 * @return 1 (true) if empty, 0 (false) otherwise.
 */
int clist_empty(const struct clist* cl);

/**
 * @brief Returns the number of items in the list.
 * @return Size of the list.
 */
size_t clist_size(const struct clist* cl);

/** @} */ // End of Inspection group

/**
 * @brief Sets the cursor. Passing NULL to new_cursor causes clist to lose the list
 */
void set_clist_cursor(struct clist* cl, struct clist_item* new_cursor);

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // CLIST_H