#ifndef ARRAYS_ARRAY_H
#define ARRAYS_ARRAY_H

#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Array API
 * 
 * @brief Very simple array wrapper around built-in arrays.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct array *arr` must be non-NULL
 * @{
 */

/**
 * @struct array
 * 
 * @brief Very simple array wrapper.
 * 
 * @warning **Buffer Ownership**: You provide buffer. So do not
 * export arrays whose buffers were allocated in the stack.
 * @warning **NULL Buffer**: BUffer cannot be NULL. You are requested
 * to provide a non-NULL array with a matching size and obj_size.
 * otherwise assertions will fail.
 */
struct array {
    void        *buffer;
    size_t      size;
    size_t      obj_size;
};

/**
 * @name Implementation
 * Some inline utilities for array.
 * @{
 */

#define ARRAY_VIEW(static_array) \
    (struct array){ .buffer = (static_array), .size = sizeof(static_array)/sizeof((static_array)[0]), .obj_size = sizeof((static_array)[0]) }

// Usage:
// int raw[] = {1, 2, 3};
// struct array view = ARRAY_VIEW(raw);

/**
 * @brief Get pointer to element at index.
 */
static inline void *array_at(const struct array *arr, size_t index)
{
    assert(arr != NULL && arr->buffer != NULL);
    assert(index < arr->size);
    return (void*) ((char*) arr->buffer + index * arr->obj_size);
}

/**
 * @return Number of elements in the array.
 */
static inline size_t array_size(const struct array* arr)
{
    assert(arr != NULL && arr->buffer != NULL);
    return arr->size;
}

/**
 * @return Object size (stride).
 */
static inline size_t array_obj_size(const struct array* arr)
{
    assert(arr != NULL && arr->buffer != NULL);
	return arr->obj_size;
}

/**
 * @return iterator at @p index.
 * 
 * @warning **Array Bounds**: Bound checks are done by assert, you
 * can obtain an iterator if max index is dynarray_size(arr).
 */
static inline void *array_iterator_at(struct array *arr, size_t index)
{
    assert(arr != NULL && arr->buffer != NULL);
    assert(index <= array_size(arr));
    return (void *) ((char *) arr->buffer + index * arr->obj_size);
}

/**
 * @return begin iterator.
 */
static inline void *array_iterator_begin(struct array *arr)
{
    assert(arr != NULL && arr->buffer != NULL);
    return arr->buffer;
}

/**
 * @return end iterator.
 */
static inline void *array_iterator_end(struct array *arr)
{
    assert(arr != NULL && arr->buffer != NULL);
    return array_iterator_at(arr, array_size(arr));
}

/**
 * @return next iterator.
 * 
 * @warning passing end iterator will invalidate assert.
 */
static inline void *array_iterator_next(struct array *arr, void *current)
{
    assert(arr != NULL && arr->buffer != NULL);
    assert(current != NULL && (array_iterator_begin(arr) <= current && current < array_iterator_end(arr)));
    return (char *) current + arr->obj_size;
}

/**
 * @return prev iterator.
 * 
 * @warning passing begin iterator will invalidate assert.
 */
static inline void *array_iterator_prev(struct array *arr, void *current)
{
    assert(arr != NULL && arr->buffer != NULL);
    assert(current != NULL && (array_iterator_begin(arr) < current && current <= array_iterator_end(arr)));
    return (char *) current - ((long) arr->obj_size);
}

/** @} */ // End of Implementation group

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // ARRAYS_ARRAY_H