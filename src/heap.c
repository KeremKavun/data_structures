#include "../include/heap.h"
#include "../../buffers/include/lbuffer.h"
#include <stdlib.h>

struct heap
{
    struct lbuffer* buffer;
    int (*cmp) (const void* a, const void* b);
};

struct context_wrapper
{
    void* userdata;
    void (*handler) (void* item, void* userdata);
};

static void reheap_up(struct heap* root, size_t index);
static void reheap_down(struct heap* root, size_t index);
static size_t parent_index(size_t index);
static size_t left_child_index(size_t index);
static size_t right_child_index(size_t index);
static void swap(void* a, void* b);
static int copy_void_ptr(const void* new_item, void* buffer_item, void* userdata);
static void exec_void_ptr(void* item, void* userdata);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct heap* heap_create(char* stack_ptr, size_t capacity, int resize, int (*cmp) (const void* a, const void* b))
{
    struct heap* tree = malloc(sizeof(struct heap));
    if (!tree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for heap");
        return NULL;
    }
    tree->buffer = lbuffer_create(stack_ptr, capacity, sizeof(void*), resize);
    if (!tree->buffer)
    {
        LOG(LIB_LVL, CERROR, "Failed to create lbuffer for heap");
        free(tree);
        return NULL;
    }
    tree->cmp = cmp;
    return tree;
}

void heap_destroy(struct heap* tree, void* context, void (*deallocator) (void* item, void* context))
{
    struct context_wrapper del_ctx = {context, deallocator};
    lbuffer_destroy(tree->buffer, &del_ctx, exec_void_ptr);
    free(tree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int heap_add(struct heap* tree, void* new_data)
{
    if (lbuffer_insert(tree->buffer, new_data, heap_size(tree), NULL, copy_void_ptr) != 0)
    {
        LOG(LIB_LVL, CERROR, "Could not inserted into the buffer");
        return 1;
    }
    reheap_up(tree, heap_size(tree) - 1);
    return 0;
}

void* heap_remove(struct heap* tree)
{
    if (heap_empty(tree))
        return NULL;
    void* del;
    void** root_data = lbuffer_at(tree->buffer, 0);
    void** last_item_data = lbuffer_at(tree->buffer, lbuffer_size(tree->buffer) - 1);
    del = *root_data;
    *root_data = *last_item_data;
    lbuffer_remove(tree->buffer, lbuffer_size(tree->buffer) - 1, NULL);
    reheap_down(tree, 0);
    return del;
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int heap_empty(const struct heap* tree)
{
    return lbuffer_empty(tree->buffer);
}

size_t heap_size(const struct heap* tree)
{
    return lbuffer_size(tree->buffer);
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void heap_walk(struct heap* tree, void* userdata, void (*handler) (void* data, void* userdata))
{
    struct context_wrapper ctx = {userdata, handler};
    lbuffer_walk(tree->buffer, &ctx, exec_void_ptr);
}

// *** Helper functions *** //

static void reheap_up(struct heap* root, size_t index)
{
    while (index > 0)
    {
        size_t p_index = parent_index(index);
        void* child_slot = lbuffer_at(root->buffer, index);
        void* parent_slot = lbuffer_at(root->buffer, p_index);
        if (root->cmp(*(void**) parent_slot, *(void**) child_slot) >= 0)
            break;
        swap(child_slot, parent_slot);
        index = p_index;
    }
}

static void reheap_down(struct heap* root, size_t index)
{
    while (1)
    {
        size_t left = left_child_index(index);
        size_t right = right_child_index(index);
        size_t largest = index;
        if (left < heap_size(root))
        {
            void* left_val = *(void**)lbuffer_at(root->buffer, left);
            void* largest_val = *(void**)lbuffer_at(root->buffer, largest);
            
            if (root->cmp(largest_val, left_val) < 0)
                largest = left;
        }
        if (right < heap_size(root))
        {
            void* right_val = *(void**)lbuffer_at(root->buffer, right);
            void* largest_val = *(void**)lbuffer_at(root->buffer, largest);
            if (root->cmp(largest_val, right_val) < 0)
                largest = right;
        }
        if (largest == index)
            break;
        swap(lbuffer_at(root->buffer, index), lbuffer_at(root->buffer, largest));
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

static void swap(void* a, void* b)
{
    void* temp = *(void**)a;
    *(void**)a = *(void**)b;
    *(void**)b = temp;
}

static int copy_void_ptr(const void* new_item, void* buffer_item, void* userdata)
{
    LOG(LIB_LVL, CINFO, "Copying %p into buffer at %p", new_item, buffer_item);
    *(void**)buffer_item = (void*)new_item;
    (void) userdata;
    return 0;
}

static void exec_void_ptr(void* item, void* userdata)
{
    struct context_wrapper* ctx = (struct context_wrapper*)userdata;
    if (ctx->handler)
        ctx->handler(*(void**)item, ctx->userdata);
}