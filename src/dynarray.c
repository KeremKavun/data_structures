#include "../include/dynarray.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Helper functions declarations and macro definitions
 */

#define ptr_add(ptr, bytes) ((void*)((char*)(ptr) + (bytes)))
#define ptr_sub(ptr1, ptr2) ((size_t)((char*)(ptr1) - (char*)(ptr2)))

static int dynarray_realloc(struct dynarray *arr, size_t new_capacity);
static int dynarray_grow(struct dynarray *arr, size_t new_size);

/* =========================================================================
 * Initialization & Deinitialization
 * ========================================================================= */

int dynarray_init(struct dynarray *arr, size_t capacity, size_t obj_size, struct object_concept oc)
{
    assert(arr != NULL && capacity != 0);
    void *buffer = malloc(capacity * obj_size);
    if (!buffer) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for dynarray buffer");
        return 1;
    }
    arr->base.buffer = buffer;
    arr->capacity = capacity;
    arr->base.size = 0;
    arr->base.obj_size = obj_size;
    arr->oc = oc;
    return 0;
}

void dynarray_deinit(struct dynarray *arr)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    if (arr->base.buffer) {
        // Deinit all inner objects
        for (size_t i = 0; i < arr->base.size; i++) {
            void *item_ptr = &((char *) arr->base.buffer)[i * arr->base.obj_size];
            arr->oc.deinit(item_ptr);
        }
        free(arr->base.buffer);
        arr->base.buffer = NULL;
    }
}

/* =========================================================================
 * Insertion
 * ========================================================================= */

int dynarray_insert(struct dynarray *arr, size_t index, void *begin, void *end)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    assert(begin != NULL && end != NULL);
    assert(index <= dynarray_size(arr));
    size_t byte_diff = ptr_sub(end, begin);
    size_t obj_count = byte_diff / arr->base.obj_size;
    // EDGE CASE CHECK: Is the source range inside the current array?
    void *buffer_start = dynarray_iterator_begin(arr);
    void *buffer_end = dynarray_iterator_end(arr);
    int is_self_insert = (begin >= buffer_start && begin < buffer_end);
    size_t offset_from_start = 0;
    if (is_self_insert)
        offset_from_start = ptr_sub(begin, buffer_start);
    if (dynarray_grow(arr, dynarray_size(arr) + obj_count) != 0)
        return 1;
    // IF we reallocated and it was a self-insert, update begin/end pointers
    if (is_self_insert && arr->base.buffer != buffer_start) {
        begin = ptr_add(arr->base.buffer, offset_from_start);
        end = ptr_add(begin, byte_diff); // We dont need this actaully anymore.
    }
    void *gap_start = ptr_add(arr->base.buffer, index * arr->base.obj_size);
    void *dest_begin = gap_start;
    void *dest_end = ptr_add(dest_begin, byte_diff);
    size_t bytes_to_move = (dynarray_size(arr) - index) * arr->base.obj_size;
    memmove(dest_end, dest_begin, bytes_to_move);
    while (dest_begin != dest_end) {
        if (is_self_insert && begin >= gap_start)
            begin = ptr_add(begin, byte_diff);
        if (arr->oc.init(dest_begin, begin) != 0) {
            size_t bytes_done = ptr_sub(dest_begin, ptr_add(arr->base.buffer, index * arr->base.obj_size));
            size_t j = bytes_done / arr->base.obj_size;
            while (j-- > 0) {
                dest_begin = ptr_add(dest_begin, -((long)arr->base.obj_size));
                arr->oc.deinit(dest_begin);
            }
            memmove(dest_begin, dest_end, bytes_to_move);
            dynarray_shrink_to_fit(arr);
            return 1;
        }
        begin = ptr_add(begin, arr->base.obj_size);
        dest_begin = ptr_add(dest_begin, arr->base.obj_size);
    }
    arr->base.size += obj_count;
    return 0;
}

int dynarray_push_back(struct dynarray *arr, void *new_data)
{
    void *end_ptr = ptr_add(new_data, arr->base.obj_size);
    return dynarray_insert(arr, dynarray_size(arr), new_data, end_ptr);
}

int dynarray_set(struct dynarray *arr, size_t index, void *value)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    assert(index < dynarray_size(arr));
    void *target = dynarray_iterator_at(arr, index);
    arr->oc.deinit(target);
    if (arr->oc.init(target, value) != 0) {
        memset(target, 0, arr->base.obj_size); 
        return 1;
    }
    return 0;
}

void dynarray_delete(struct dynarray *arr, size_t begin, size_t end)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    size_t obj_count = end - begin;
    assert(end <= dynarray_size(arr));
    assert(dynarray_size(arr) >= obj_count);
    void *dest = dynarray_iterator_at(arr, begin);
    void *target_begin = dest;
    void *target_end = dynarray_iterator_at(arr, end);
    size_t bytes_to_move = (dynarray_size(arr) - end) * arr->base.obj_size;
    while (target_begin != target_end) {
        arr->oc.deinit(target_begin);
        target_begin = dynarray_iterator_next(arr, target_begin);
    }
    memmove(dest, target_end, bytes_to_move);
    arr->base.size -= obj_count;
}

void dynarray_pop_back(struct dynarray *arr)
{
    dynarray_delete(arr, dynarray_size(arr) - 1, dynarray_size(arr));
}

/* =========================================================================
 * Capacity & Size
 * ========================================================================= */

int dynarray_reserve(struct dynarray *arr, size_t new_cap)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    if (new_cap <= dynarray_capacity(arr))
        return 0;
    return dynarray_realloc(arr, new_cap);
}

int dynarray_shrink_to_fit(struct dynarray *arr)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    if (arr->capacity == dynarray_size(arr))
        return 0;
    return dynarray_realloc(arr, dynarray_size(arr));
}

void dynarray_clear(struct dynarray *arr)
{
    dynarray_delete(arr, 0, dynarray_size(arr));
}

int dynarray_resize(struct dynarray *arr, size_t new_size, void *default_val)
{
    size_t current_size = dynarray_size(arr);
    if (new_size < current_size) {
        dynarray_delete(arr, new_size, current_size);
    } else if (new_size > current_size) {
        if (dynarray_reserve(arr, new_size) != 0)
            return 1;
        void *iter = dynarray_iterator_at(arr, current_size);
        size_t items_to_add = new_size - current_size;
        for (size_t i = 0; i < items_to_add; ++i) {
            // If this operation to fail, then it cant fail after a
            // successfull operation, since they all take same def val?
            if (arr->oc.init(iter, default_val) != 0) {
                arr->base.size = current_size + i;
                return 1;
            }
            iter = (char*)iter + arr->base.obj_size;
        }
        arr->base.size = new_size;
    }
    return 0;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

int dynarray_get(const struct dynarray *arr, size_t index, void *result)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    if (index >= dynarray_size(arr))
    {
        LOG(LIB_LVL, CERROR, "Attempt to get out of bounds");
        return 1;
    }
    memcpy(result, dynarray_iterator_at((struct dynarray *) arr, index), arr->base.obj_size);
    return 0;
}

void *dynarray_front(struct dynarray *arr)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    return arr->base.buffer;
}

void *dynarray_back(struct dynarray *arr)
{
    return dynarray_iterator_at(arr, dynarray_size(arr) - 1);
}

/* =========================================================================
 * Iterators
 * ========================================================================= */

// *** Helper functions definitions *** //

static int dynarray_realloc(struct dynarray *arr, size_t new_capacity)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    char* new_buffer = realloc(arr->base.buffer, new_capacity * arr->base.obj_size);
    if (!new_buffer) {
        LOG(LIB_LVL, CERROR, "Reallocation failed, could not grow the buffer");
        return 1;
    }
    arr->base.buffer = new_buffer;
    arr->capacity = new_capacity;
    return 0;
}

static int dynarray_grow(struct dynarray *arr, size_t new_size)
{
    assert(arr != NULL && arr->base.buffer != NULL);
    if (new_size > arr->capacity) {
        size_t new_cap = dynarray_capacity(arr);
        while (new_cap < new_size)
            new_cap *= 2;
        return dynarray_realloc(arr, new_cap);
    }
    return 0;
}