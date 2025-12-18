#include "../include/Btree.h"
#include "../../stack/include/bstack.h"
#include <stdlib.h>

/*
    Btree nodes are classic mway node concept introduced in mwaytree.h + first node + size_t size to track the count of data
    allocated in the footer area. Here how it look like:

    struct mway_header;
    struct mway_entry[N];
    struct mway_header* first_child;
    size_t size;
*/

/*
    Assuming no duplicate is passed for now.
*/

struct Btree
{
    struct mway_header* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

// Insert node at size_t index given by search_node. If index == (size_t) -1, insert into the first entry, if 0, i insert 1, in short, insert index + 1.
// Return entry with NULLs if not overflowed else return last entry (save before shift). This function is noexcept
static struct mway_entry insert_data(struct mway_header* node, struct mway_entry new_entry, size_t index);
// Split node by creating a new node and copying ((capacity / 2) - 1) into the new node and add last_entry to the new node.
// After split, original node ptr is still preserved, no need to set, as asserted in insert_data. Return median as last entry of the original node.
// In case of failure, return mway_entry with NULLs.
static struct mway_entry split_node(struct mway_header* node, struct mway_entry last_entry, struct object_concept* oc);
// Check if deletion would cause underflow, if so save chiild ptr of entry to be deleted, else set the ptr to be returned to NULL.
// size_t index is zero indexed provided by search_node
// shift remaining at the right to the left
// Return the ptr. NULL ptr will be interpreted as no underflow, while non NULL is vice versa
static struct mway_header* delete_data(struct mway_header* node, size_t index, struct object_concept* oc);
static void merge_nodes(struct mway_header* node_a, struct mway_header* node_b, struct object_concept* oc);

// Return the index of previos entry if current entry data is greater. If no entry like that, return size - 1 of the nodes.
// Note (size_t) -1 is returned if first entries data is greater than the input data. Notice the input data is either in data field
// or in child nodes of entries child in returned index.
static size_t search_node(struct mway_header* node, const void* data, int (*cmp) (const void* key, const void* data));
static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context));
static struct mway_header* get_node_first_child(const struct mway_header* node);
static size_t get_node_size(const struct mway_header* node);
static struct mway_header** get_node_first_child_ptr(struct mway_header* node);
static size_t* get_node_size_ptr(struct mway_header* node);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct Btree* Btree_create(size_t order, int (*cmp) (const void* key, const void* data), struct object_concept* oc)
{
    struct Btree* tree = malloc(sizeof(struct Btree));
    if (!tree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    struct mway_header* root = mway_create(order - 1, sizeof(struct mway_header*) + sizeof(size_t), oc);
    if (!root)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate B-tree root");
        return NULL;
    }
    tree->root = root;
    tree->oc = oc;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}

void Btree_destroy(struct Btree* tree, void* context)
{
    mway_destroy(tree->root, context, tree->oc);
    free(tree);
}

size_t Btree_node_sizeof(size_t order)
{
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    return mway_sizeof(order - 1, sizeof(struct mway_header*) + sizeof(size_t));
}
 
/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status Btree_add(struct Btree* tree, void* new_data)
{
    enum trees_status status = TREES_SYSTEM_ERROR;
    // Anonymous struct to copy nodes and index together into the stack
    struct
    {
        struct mway_header* node;
        size_t index;
    } buffer;
    struct bstack bs;
    // Initialize stack to store indexes of traversed nodes
    if (bstack_init(&bs, NULL, sizeof(buffer)) != 0)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate stack for backtracing");
        goto fail_stack;
    }
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        size_t index = search_node(curr, new_data, tree->cmp);
        buffer.node = curr;
        buffer.index = index;
        if (bpush(&bs, &buffer, NULL, NULL) != 0)
        {
            LOG(LIB_LVL, CERROR, "Could not push the buffer");
            goto fail_push;
        }
        // Update curr. If index is (size_t) -1, proceed with first child
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    struct mway_entry curr_entry = {.data = new_data, .child = NULL};
    // Keep going until either root or no overflow or split node raises errors by setting curr_entry to NULLs
    while (!bstack_empty(&bs) && (curr_entry.data != NULL || curr_entry.child != NULL))
    {
        // noexcept since using POD
        bpop(&bs, &buffer);
        // index=-1 -> i=0, index=0 -> i=1, etc.
        // Works due to unsigned wraparound when index=(size_t)-1
        curr_entry = insert_data(buffer.node, curr_entry, buffer.index + 1);
        // No overflow means no split and no median propagation upwards
        if (curr_entry.data == NULL && curr_entry.child == NULL)
            break;
        curr_entry = split_node(buffer.node, curr_entry, tree->oc);
        // split_node failed and returned NULL entry
        if (curr_entry.data == NULL && curr_entry.child == NULL)
        {
            bstack_deinit(&bs, NULL, NULL);
            goto fail_split;
        }
    }
    // new root needs to be created
    if (curr_entry.data != NULL || curr_entry.child != NULL)
    {
        struct mway_header* new_root = mway_create(tree->root->capacity, sizeof(struct mway_header*) + sizeof(size_t), tree->oc);
        if (!new_root)
        {
            LOG(LIB_LVL, CERROR, "Could not allocate new B-tree root");
            goto fail_root;
        }
        struct mway_header* tmp = tree->root;
        tree->root = new_root;
        // Set first child to previous root, which was remained in the left after the last split
        *get_node_first_child_ptr(tree->root) = tmp;
        mway_set_entry(tree->root, 0, &curr_entry);
        *get_node_size_ptr(tree->root) = 1;
        // Result should be a root with first child old root and median is first entry
    }
    status = TREES_OK;
    fail_root:
    fail_split:
    fail_push:
    bstack_deinit(&bs, NULL, NULL);
    fail_stack:
    return status;
}

void* Btree_remove(struct Btree* tree, void* data)
{
    void* ref = NULL;
    // Anonymous struct to copy nodes and index together into the stack
    struct
    {
        struct mway_header* node;
        size_t index;
    } buffer;
    struct bstack bs;
    // Initialize stack to store indexes of traversed nodes
    if (bstack_init(&bs, NULL, sizeof(buffer)) != 0)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate stack for backtracing");
        goto fail_stack;
    }
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        size_t index = search_node(curr, data, tree->cmp);
        buffer.node = curr;
        buffer.index = index;
        // Found the data, stop traversing the tree.
        if (index != (size_t) -1 && tree->cmp(mway_get_data_const(curr, index), data) == 0)
            break;
        if (bpush(&bs, &buffer, NULL, NULL) != 0)
        {
            LOG(LIB_LVL, CERROR, "Could not push the buffer");
            goto fail_push;
        }
        // Update curr. If index is (size_t) -1, proceed with first child
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    // Could not find the data
    if (curr == NULL)
    {
        LOG(LIB_LVL, CERROR, "Could not find the data to remove");
        goto fail_find;
    }
    struct mway_header* curr_node = delete_data(buffer.node, buffer.index, tree->oc);
    while (!bstack_empty(&bs))
    {

    }
    ref = NULL;
    fail_find:
    fail_push:
    bstack_deinit(&bs, NULL, NULL);
    fail_stack:
    return ref;
}

void* Btree_search(struct Btree* tree, const void* data)
{
    void* result;
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        size_t index = search_node(curr, data, tree->cmp);
        if (index == (size_t) -1)
        {
            curr = get_node_first_child(curr);
            continue;
        }
        // Doing a comparison again, need optimization
        if (tree->cmp(mway_get_data_const(curr, index), data) == 0)
            return result;
        curr = mway_get_child(curr, index);
    }
    return NULL;
}
 
/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const struct mway_header* Btree_root(const struct Btree* tree)
{
    return tree->root;
}

int Btree_empty(const struct Btree* tree)
{
    return tree->root == NULL;
}

size_t Btree_size(const struct Btree* tree)
{
    return tree->size;
}

size_t Btree_order(const struct Btree* tree)
{
    return tree->root->capacity + 1;
}
 
/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/
 
void Btree_walk(struct Btree* tree, void* context, void (*handler) (void* data, void* context))
{
    walk_helper(tree->root, context, handler);
}

// *** Helper functions *** //

static struct mway_entry insert_data(struct mway_header* node, struct mway_entry new_entry, size_t index)
{
    size_t* size_ptr = get_node_size_ptr(node);
    size_t size = *size_ptr;
    // Insert struct mway_entry* new_entry and return last
    if (size == node->capacity)
    {
        if (index < size)
        {
            struct mway_entry last_entry = mway_get_entry(node, node->capacity - 1);
            // i is where we encounter first data that is bigger than new_entries data. We want to insert here, thus, shifting by 1.
            // dest is i + 1, src is i. i is zero indexed while size is not, so we use size - i gives data num from i to the last
            // instead of size - i + 1. But we need to substract 1 too, because we will discard the last data.
            memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (node->capacity - index - 1));
            mway_set_entry(node, index, &new_entry);
            return last_entry;
        }
        else
            return new_entry;
    }
    // This time, we dont substract size_t __n by one, since we have space
    // If i is size, we are adding to the very end and memmove shouldnt throw error, because 0 element is requested to be moved
    memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (size - index));
    mway_set_entry(node, index, &new_entry);
    (*size_ptr)++;
    return (struct mway_entry) {.data = NULL, .child = NULL};
}

static struct mway_entry split_node(struct mway_header* node, struct mway_entry last_entry, struct object_concept* oc)
{
    struct mway_header* new_node = mway_create(node->capacity, sizeof(struct mway_header*) + sizeof(size_t), oc);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate new B-tree node");
        return (struct mway_entry) {.data = NULL, .child = NULL};
    }
    size_t median_index = node->capacity / 2;
    struct mway_entry median = mway_get_entry(node, median_index);
    // Move data AFTER median to new_node. Start copying from median_index + 1
    size_t copy_start_index = median_index + 1;
    size_t entries_from_node = node->capacity - copy_start_index;
    // But what will be the first child of the new_node? simply last child of the original node, two nodes pointing same node??
    if (entries_from_node > 0)
        memcpy(mway_get_entry_addr(new_node, 0), mway_get_entry_addr(node, copy_start_index), sizeof(struct mway_entry) * entries_from_node);
    // Add the 'last_entry' (overflow) to the end of new_node
    mway_set_entry(new_node, entries_from_node, &last_entry);
    // New node has (items_from_node) + 1 (last_data)
    *get_node_size_ptr(new_node) = entries_from_node + 1;
    // Old node is reduced to just the items before the median
    *get_node_size_ptr(node) = median_index;
    // This is critical
    // First set first child of new_node to median.child, because this pointer is responsible for storing nodes greater than median
    // and less than median + 1
    *get_node_first_child_ptr(new_node) = median.child;
    // Then set median.child to new_node. Sanity check: we just allocated one node, parent nodes node pointing 'node' still points to original
    // which is correct, since it holds values less than median.
    median.child = new_node;
    return median;
}

static struct mway_header* delete_data(struct mway_header* node, size_t index, struct object_concept* oc)
{
    struct mway_header* right_child = NULL;
    size_t* size_ptr = get_node_size_ptr(node);
    size_t size = *size_ptr;
    // Deletion will going to cause underflow, save the child of entry into right_child
    if (size <= ((node->capacity + 2) / 2) - 1)
        right_child = mway_get_child(node, index);
    // Destruct entries data
    if (oc->destruct)
        oc->destruct(mway_get_data(node, index), NULL);
    // Shifth the buffer, deleting i, moving i + 1 into i, since i is 0 indexed, substracting -1 one more time from (size - i) to get the count of remaining
    // 0 elements are handled naturally, causing memmove to take 0 as the third parameter
    memmove(mway_get_entry_addr(node, index), mway_get_entry_addr(node, index + 1), sizeof(struct mway_entry) * (size - (index + 1)));
    (*size_ptr)--;
    return right_child;
}

static void merge_nodes(struct mway_header* node_a, struct mway_header* node_b, struct object_concept* oc)
{

}

static size_t search_node(struct mway_header* node, const void* data, int (*cmp) (const void* key, const void* data))
{
    size_t size = get_node_size(node);
    for (size_t i = 0; i < size; i++)
    {
        int result = cmp(mway_get_data_const(node, i), data);
        if (result > 0)
            return i - 1;
    }
    return size - 1;
}

static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context))
{
    if (!root)
        return;
    size_t size = get_node_size(root);
    walk_helper(get_node_first_child(root), context, handler);
    for (size_t i = 0; i < size; i++)
    {
        handler(mway_get_data(root, i), context);
        walk_helper(mway_get_child(root, i), context, handler);
    }
}

static struct mway_header* get_node_first_child(const struct mway_header* node)
{
    return *(struct mway_header**) mway_get_footer(node);
}

static size_t get_node_size(const struct mway_header* node)
{
    return *(size_t*) ((char*) mway_get_footer_const(node) + ALIGN_REQ);
}

static struct mway_header** get_node_first_child_ptr(struct mway_header* node)
{
    return (struct mway_header**) mway_get_footer(node);
}

static size_t* get_node_size_ptr(struct mway_header* node)
{
    return (size_t*) ((char*) mway_get_footer(node) + ALIGN_REQ);
}