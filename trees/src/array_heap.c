#include "../include/array_heap.h"
#include "../../arrays/include/dynarray.h"
#include <stdlib.h>
#include <inttypes.h>

#define INITIAL_CAPACITY 4

static void reheap_up(struct array_heap* root, size_t index);
static void reheap_down(struct array_heap* root, size_t index);
static size_t parent_index(size_t index);
static size_t left_child_index(size_t index);
static size_t right_child_index(size_t index);
static void swap(void* a, void* b, struct dynarray *arr);

/* =========================================================================
 * Initialize & Deinitialize
 * ========================================================================= */

int array_heap_init(struct array_heap* tree, size_t obj_size, struct object_concept *oc, int (*cmp) (const void* a, const void* b))
{
    if (dynarray_init(&tree->contents, INITIAL_CAPACITY, obj_size, *oc) != 0) {
        LOG(LIB_LVL, CERROR, "Failed to create dynarray for array_heap");
        return 1;
    }
    tree->cmp = cmp;
    return 0;
}

void array_heap_deinit(struct array_heap* tree)
{
    dynarray_deinit(&tree->contents);
    tree->cmp = NULL;
}

/* =========================================================================
 * Operations
 * ========================================================================= */

int array_heap_add(struct array_heap* tree, void* new_data)
{
    assert(tree != NULL);
    if (dynarray_push_back(&tree->contents, new_data) != 0) {
        LOG(LIB_LVL, CERROR, "Could not inserted into the buffer");
        return 1;
    }
    reheap_up(tree, array_heap_size(tree) - 1);
    return 0;
}

int array_heap_remove(struct array_heap* tree, void *removed)
{
    assert(tree != NULL);
    if (array_heap_empty(tree))
        return 0;
    void* root_data = dynarray_front(&tree->contents);
    if (tree->contents.oc.init(removed, root_data) != 0) {
        LOG(LIB_LVL, CERROR, "Could not initialized the destination");
        return 1;
    }
    if (dynarray_set(&tree->contents, 0, dynarray_back(&tree->contents)) != 0) {
        LOG(LIB_LVL, CERROR, "Could not copy last item into the root");
        return 1;
    }
    dynarray_pop_back(&tree->contents);
    reheap_down(tree, 0);
    return 0;
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

int array_heap_empty(const struct array_heap* tree)
{
    assert(tree != NULL);
    return dynarray_empty(&tree->contents);
}

size_t array_heap_size(const struct array_heap* tree)
{
    assert(tree != NULL);
    return dynarray_size(&tree->contents);
}

// Iteration

void array_heap_walk(struct array_heap* tree, void* context, void (*handler) (void* data, void* context))
{
    assert(tree);
    void *begin = dynarray_iterator_begin(&tree->contents);
    void *end = dynarray_iterator_end(&tree->contents);
    while (begin != end) {
        handler(begin, context);
        begin = dynarray_iterator_next(&tree->contents, begin);
    }
}

// *** Helper functions *** //

static void reheap_up(struct array_heap* root, size_t index)
{
    while (index > 0) {
        size_t p_index = parent_index(index);
        void* child_slot = array_at((struct array *) &root->contents, index);
        void* parent_slot = array_at((struct array *) &root->contents, p_index);
        if (root->cmp(parent_slot, child_slot) >= 0)
            break;
        swap(child_slot, parent_slot, &root->contents);
        index = p_index;
    }
}

static void reheap_down(struct array_heap* root, size_t index)
{
    while (1) {
        size_t left = left_child_index(index);
        size_t right = right_child_index(index);
        size_t largest = index;
        if (left < array_heap_size(root)) {
            void* left_val = array_at((struct array *) &root->contents, left);
            void* largest_val = array_at((struct array *) &root->contents, largest);
            if (root->cmp(largest_val, left_val) < 0)
                largest = left;
        }
        if (right < array_heap_size(root)) {
            void* right_val = array_at((struct array *) &root->contents, right);
            void* largest_val = array_at((struct array *) &root->contents, largest);
            if (root->cmp(largest_val, right_val) < 0)
                largest = right;
        }
        if (largest == index)
            break;
        swap(array_at((struct array *) &root->contents, index), array_at((struct array *) &root->contents, largest), &root->contents);
        index = largest;
    }
}

static size_t parent_index(size_t index)
{
    return (index - 1) / 2;
}

static size_t left_child_index(size_t index)
{
    return 2 * index + 1;
}

static size_t right_child_index(size_t index)
{
    return 2 * index + 2;
}

static void swap(void* a, void* b, struct dynarray *arr)
{
    uint8_t tmp[arr->base.obj_size];
    arr->oc.init(tmp, a);
    arr->oc.init(a, b);
    arr->oc.init(b, tmp);
}