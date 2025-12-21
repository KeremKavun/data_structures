#ifndef ARRAY_DYNARRAY_H
#define ARRAY_DYNARRAY_H

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "array.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Dynamic Array API
 * 
 * @brief cpp vector like API.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct dynarray *arr` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal slots are owned by the dynarray and managed thru object_concept given by user
 * - to init and deinit objects in place.
 * @{
 */

/**
 * @struct dynarray
 * 
 * @brief Automatically growing array as you insert new items.
 * Aggregation of 
 * 
 * @note size attribute of base now refers to number of elements stored.
 */
struct dynarray {
  struct array            base;
  size_t                  capacity;
  struct object_concept   oc;
};

/**
 * @name Initialization & Deinitialization
 * Functions for setting up the dynarray.
 * @{
 */

 /**
  * @brief Initializes dynarray.
  * 
  * @param[in, out] arr Pointer to the dynarray instance.
  * @param[in] capacity Capacity of your buffer, cannot be zero.
  * @param[in] obj_size Size of object type to be stored.
  * @param[in] oc copy concept of object to init and deinit objects.
  * 
  * @return 0 in case of success, non-zero otherwise
  * 
  * @see object_concept
  */
int dynarray_init(struct dynarray *arr, size_t capacity, size_t obj_size, struct object_concept oc);

/**
 * @brief Deinitializes the dynarray.
 * 
 * @param[in, out] arr Pointer to the dynarray instance.
 * 
 * @warning **Clearing Atrtributes**: Only pointer that points to buffer
 * will be set to NULL for performance concerns, and you will get assertion
 * fail if you pass deinited dynarray, since buffer cannot be NULL (note this
 * implies capacity cannot be zero as mentioned earlier).
 */
void dynarray_deinit(struct dynarray *arr);

/** @} */ // End of Initialization & Deinitalization group

/**
 * @name Insertion & Removal
 * Functions for insertion and removal.
 * @{
 */

/**
 * @brief Inserts items at given index, from [begin, end).
 * 
 * @param[in] index Index to insert objects from.
 * @param[in] begin Pointer to the beginning of objects to insert.
 * @param[in] end Pointer to the one past the last object to insert.
 * 
 * @return 0 in success, non-zero otherwise.
 * 
 * @warning **Pointer Validity** begin and end pointers must be valid and non-NULL,
 * obtained by iterator methods such as @ref dynarray_iterator_at.
 * @warning **Object Type** begin and end iterators must point to objects with
 * same type, otherwise you will likely to run into UB or core dumped.
 * @warning **Array Boundary Checks** index check is done by assert, you will
 * get an assertion log and program crash in case index isnt correct.
 * 
 * @note This operation is **O(N)**, linear time.
 * @note This operation provides strong guarantee.
 * @note You can insert from array itself, overlapping arrays.
 */
int dynarray_insert(struct dynarray *arr, size_t index, void *begin, void *end);

/**
 * @brief Wrapper to insert single object at the end.
 * 
 * @see dynarray_insert
 */
int dynarray_push_back(struct dynarray *arr, void *new_data);

/**
 * @brief Replaces the element at @p index with @p value.
 * 
 * @return 0 on success, non-zero if index is out of bounds.
 * 
 * @warning **Failure Case**: if initialization of new object
 * fails, then that slot will be set to zero.
 * 
 * @note This safely destroys the existing object at @p index before
 * initializing it with the new @p value, ensuring no resource leaks.
 */
int dynarray_set(struct dynarray *arr, size_t index, void *value);

/**
 * @brief Deletes items, from [begin, end).
 * 
 * @param[in] begin Beginning index to delete from.
 * @param[in] end Ending index (exclusive).
 * 
 * @warning **Array Boundary Checks** begin and end check is done by assert, you will
 * get an assertion log and program crash in case indices arent correct.
 * 
 * @note This operation is **O(N)**, linear time.
 */
void dynarray_delete(struct dynarray *arr, size_t begin, size_t end);

/**
 * @brief Wrapper to delete last object.
 * 
 * @see dynarray_delete
 */
void dynarray_pop_back(struct dynarray *arr);

/** @} */ // End of Insertion & Removal group

/**
 * @name Capacity & Size
 * Functions to query dynarray and slot states.
 * @{
 */

/**
 * @brief reserve extra capacity in addition to existing
 * capacity.
 * 
 * @param[in] new_cap new capacity.
 * 
 * @return 0 on success, including @p new_cap being less
 * than existing capacity, non-zero otherwise.
 */
int dynarray_reserve(struct dynarray *arr, size_t new_cap);

/**
 * @brief Reclaim unused memory.
 * 
 * @return 0 on success, non-zero otherwise.
 */
int dynarray_shrink_to_fit(struct dynarray *arr);

/**
 * @brief Removes all elements but keeps the allocated capacity.
 * 
 * @note Destroys all objects (O(N)) but preserves the buffer for reuse.
 * Post-condition: dynarray_size(arr) == 0.
 */
void dynarray_clear(struct dynarray *arr);

/**
 * @brief Resizes the container to contain @p new_size elements.
 * 
 * @param default_val If new_size > current_size, new elements are initialized 
 * as copies of this value.
 * 
 * @return 0 on success (allocation), non-zero failure.
 */
int dynarray_resize(struct dynarray *arr, size_t new_size, void *default_val);

/** @} */ // End of Capacity & Size group

/**
 * @name Inspection
 * Functions to query dynarray and slot states.
 * @{
 */

/**
 * @brief Copy element at @p index into @p result, a
 * measure for iterator invalidation.
 * 
 * @param[in] index of object to copy.
 * @param[in] result Pointer to destination, note that dest
 * should be same type or big enough to hold. 
 * 
 * @return 0 if index is inside bounds, non-zero otherwise.
 */
int dynarray_get(const struct dynarray *arr, size_t index, void *result);

/**
 * @brief Returns pointer to the first element.
 * @warning Undefined if empty.
 */
void *dynarray_front(struct dynarray *arr);

/**
 * @brief Returns pointer to the last element.
 * @warning Undefined if empty.
 */
void *dynarray_back(struct dynarray *arr);

/**
 * @return 1 if empty, 0 otherwise.
 */
static inline int dynarray_empty(const struct dynarray *arr)
{
	return arr->base.size == 0;
}

/**
 * @return Current size (number of elements stored).
 */
static inline size_t dynarray_size(const struct dynarray *arr)
{
	return array_size((const struct array *) arr);
}

/**
 * @return Current capacity (number of elements that can be stored without realloc).
 */
static inline size_t dynarray_capacity(const struct dynarray *arr)
{
	assert(arr != NULL && arr->base.buffer != NULL);
	return arr->capacity;
}

/**
 * @return Size of object stored in dynarray.
 */
static inline size_t dynarray_obj_size(const struct dynarray *arr)
{
	return array_obj_size((const struct array *) arr);
}

/** @} */ // End of Inspection group

/**
 * @name Iterators
 * Operations to create and use slices and iterators.
 * @{
 */

/**
 * @return iterator at @p index.
 * 
 * @warning **Array Bounds**: Bound checks are done by assert, you
 * can obtain an iterator if max index is dynarray_size(arr).
 */
static inline void *dynarray_iterator_at(struct dynarray *arr, size_t index)
{
    return array_iterator_at((struct array *) arr, index);
}

/**
 * @return begin iterator.
 */
static inline void *dynarray_iterator_begin(struct dynarray *arr)
{
    return array_iterator_begin((struct array *) arr);
}

/**
 * @return end iterator.
 */
static inline void *dynarray_iterator_end(struct dynarray *arr)
{
    return array_iterator_end((struct array *) arr);
}

/**
 * @return next iterator.
 * 
 * @warning passing end iterator will invalidate assert.
 */
static inline void *dynarray_iterator_next(struct dynarray *arr, void *current)
{
    return array_iterator_next((struct array *) arr, current);
}

/**
 * @return prev iterator.
 * 
 * @warning passing begin iterator will invalidate assert.
 */
static inline void *dynarray_iterator_prev(struct dynarray *arr, void *current)
{
    return array_iterator_prev((struct array *) arr, current);
}

/** @} */ // End of Iterators group

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // ARRAY_DYNARRAY_H