#include "../include/Btree.h"
#include "../../stack/include/vstack.h"
#include <stdlib.h>
#include <assert.h>

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

struct btree_buffer {
    struct mway_header      *node;
    size_t                  index;
};

static int copy_bb(void *dest, void *src)
{
    *(struct btree_buffer *) dest = *(const struct btree_buffer *) src;
    return 0;
}
static void deinit_bb(void *ptr) {}

struct btree_node_footer_layout {
    struct mway_header      *first_child;
    size_t                  current_size;
};

/**
 * @brief Inserts a new entry into a node at a specific index, handling potential overflow.
 * * If the node has space, the entry is inserted and the trailing entries are shifted right.
 * If the node is full (size == capacity), the function returns the "overflowed" entry 
 * that was pushed out or the new entry itself if it belongs at the end.
 *
 * @param[in,out] node  Pointer to the mway node.
 * @param[in]     new_entry The entry (data and child pointer) to insert.
 * @param[in]     index The target position within the entry array.
 * 
 * @return A null-initialized @ref mway_entry if insertion succeeded without overflow.
 * The overflowed @ref mway_entry (the last entry) if the node was already full.
 * 
 * @note This function is `noexcept` and assumes the node is non-NULL.
 */
static struct mway_entry insert_data(struct mway_header* node, struct mway_entry new_entry, size_t index);

/**
 * @brief Splits a full node into two, creating a new sibling node.
 * This function handles the "Median Split" logic typical in B-Trees. It moves half of 
 * the entries to a newly allocated node and determines which entry should be 
 * promoted to the parent.
 *
 * @param[in,out] node Pointer to the full node to be split.
 * @param[in]     last_entry The entry that caused the overflow in @ref insert_data.
 * @param[in]     ac The allocator concept used to create the new sibling node.
 * 
 * @return The median @ref mway_entry to be promoted to the parent node.
 * A null-initialized @ref mway_entry on allocation failure.
 */
static struct mway_entry split_node(struct mway_header* node, struct mway_entry last_entry, struct allocator_concept *ac);

/**
 * @brief Just save the entry to be removed and then shift.
 * 
 * @param[in, out] node Node to remove the entry.
 * @param[in] index Index of entry to be removed.
 * 
 * @return Removed entry from the B-tree.
 */
static struct mway_entry remove_data(struct mway_header *node, size_t index);

static void borrow_from_left(struct mway_header *donor, struct mway_entry *entry);
static void borrow_from_right(struct mway_header *starving, struct mway_entry *entry);

// Return the index of previos entry if current entry data is greater. If no entry like that, return size - 1 of the nodes.
// Note (size_t) -1 is returned if first entries data is greater than the input data. Notice the input data is either in data field
// or in child nodes of entries child in returned index.

/**
 * @brief Searches for the appropriate child index to traverse for a given data key.
 * Performs a linear scan to find the first entry where the node data is strictly 
 * greater than the input @p data.
 * 
 * @param[in] node The node to search.
 * @param[in] data The key/data to search for.
 * @param[in] cmp  Comparison function (returns >0 if key > data).
 * 
 * @return The index of the entry whose child pointer should be followed.
 * (size_t)-1 if the input data is smaller than the very first entry 
 * (implies traversal should follow the "first child" stored in the footer).
 * (size_t)(size - 1) if the input data is greater than or equal to all entries.
 */
static size_t search_node(struct mway_header* node, const void* data, int (*cmp) (const void* key, const void* data));

/**
 * @brief Recursive helper for in-order traversal of the tree.
 * Visits the "first child" (footer), then iterates through entries, visiting 
 * data and then the subsequent child.
 * 
 * @param[in] root The current node.
 * @param[in] context User-provided context passed to the handler.
 * @param[in] handler Callback function to process each data item.
 */
static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context));

/** 
 * @brief Gets the "leftmost" child pointer stored in the node's footer.
 * @note This pointer handles the range of values strictly less than entry[0].
 */
static struct mway_header* get_node_first_child(const struct mway_header* node);

/** 
 * @brief Gets the current number of active entries (size) from the footer.
 * @note This is distinct from capacity. Capacity is fixed; size varies.
 */
static size_t get_node_size(const struct mway_header* node);

/** @return Mutable pointer to the "leftmost" child field in the footer. */
static struct mway_header** get_node_first_child_ptr(struct mway_header* node);

/** @return Mutable pointer to the size field in the footer. */
static size_t* get_node_size_ptr(struct mway_header* node);

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct Btree *Btree_create(size_t order, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac)
{
    struct Btree* tree = malloc(sizeof(struct Btree));
    if (!tree) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    struct mway_header* root = mway_create(order - 1, sizeof(struct btree_node_footer_layout), ac);
    if (!root) {
        LOG(LIB_LVL, CERROR, "Could not allocate B-tree root");
        return NULL;
    }
    tree->root = root;
    tree->ac = *ac;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}

void Btree_destroy(struct Btree *tree, struct object_concept *oc)
{
    mway_destroy(tree->root, oc, &tree->ac);
    free(tree);
}

/* =========================================================================
 * Operations
 * ========================================================================= */

enum trees_status Btree_add(struct Btree* tree, void* new_data)
{
    enum trees_status status = TREES_SYSTEM_ERROR;
    // Anonymous struct to copy nodes and index together into the stack
    struct btree_buffer buffer;
    struct object_concept oc = { .init = copy_bb, .deinit = deinit_bb };
    struct vstack bs;
    // Initialize stack to store indexes of traversed nodes
    if (vstack_init(&bs, sizeof(buffer), &oc) != 0) {
        LOG(LIB_LVL, CERROR, "Could not allocate stack for backtracing");
        goto fail_stack;
    }
    struct mway_header* curr = tree->root;
    while (curr != NULL) {
        size_t index = search_node(curr, new_data, tree->cmp);
        buffer.node = curr;
        buffer.index = index;
        if (vpush(&bs, &buffer) != 0) {
            LOG(LIB_LVL, CERROR, "Could not push the buffer");
            goto fail_push;
        }
        // Update curr. If index is (size_t) -1, proceed with first child
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    struct mway_entry curr_entry = {.data = new_data, .child = NULL};
    // Keep going until either root or no overflow or split node raises errors by setting curr_entry to NULLs
    while (!vstack_empty(&bs) && (curr_entry.data != NULL || curr_entry.child != NULL)) {
        // noexcept since using POD
        vpop(&bs, &buffer);
        // index=-1 -> i=0, index=0 -> i=1, etc.
        // Works due to unsigned wraparound when index=(size_t)-1
        curr_entry = insert_data(buffer.node, curr_entry, buffer.index + 1);
        // No overflow means no split and no median propagation upwards
        if (curr_entry.data == NULL && curr_entry.child == NULL)
            break;
        curr_entry = split_node(buffer.node, curr_entry, &tree->ac);
        // split_node failed and returned NULL entry
        if (curr_entry.data == NULL && curr_entry.child == NULL)
            goto fail_split;
    }
    // new root needs to be created
    if (curr_entry.data != NULL || curr_entry.child != NULL) {
        struct mway_header* new_root = mway_create(tree->root->capacity, sizeof(struct btree_node_footer_layout), &tree->ac);
        if (!new_root) {
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
    vstack_deinit(&bs);
    fail_stack:
    return status;
}

static int is_underflowed(struct mway_header *node)
{
    size_t m = node->capacity + 1;
    size_t min_entries = (m + 1) / 2 - 1; // Standard integer ceiling for (m/2) - 1
    return get_node_size(node) < min_entries;
}

void* Btree_remove(struct Btree* tree, void* data)
{
    void* ref = NULL;
    // Anonymous struct to copy nodes and index together into the stack
    struct btree_buffer buffer;
    struct object_concept oc = { .init = copy_bb, .deinit = deinit_bb };
    struct vstack bs;
    // Initialize stack to store indexes of traversed nodes
    if (vstack_init(&bs, sizeof(buffer), &oc) != 0) {
        LOG(LIB_LVL, CERROR, "Could not allocate stack for backtracing");
        goto fail_stack;
    }
    struct mway_header* curr = tree->root;
    while (curr != NULL) {
        size_t index = search_node(curr, data, tree->cmp);
        buffer.node = curr;
        buffer.index = index;
        // Found the data, stop traversing the tree.
        if (index != (size_t) -1 && tree->cmp(mway_get_data_const(curr, index), data) == 0)
            break;
        if (vpush(&bs, &buffer) != 0) {
            LOG(LIB_LVL, CERROR, "Could not push the buffer");
            goto fail_push;
        }
        // Update curr. If index is (size_t) -1, proceed with first child
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    // Could not find the data
    if (curr == NULL) {
        LOG(LIB_LVL, CERROR, "Could not find the data to remove");
        goto fail_find;
    }
    struct mway_entry removed = remove_data(buffer.node, buffer.index);
    while (!vstack_empty(&bs)) {
        // Early exit
        if (!is_underflowed(buffer.node))
            break;
        // Get parent of the node in which a ref removed
        vpop(&bs, &buffer);
        // Notice we should somehow fix the Btree, since is_underflowed returned true
        // First borrow from left, buffers index - 1 will give right header to borrow over
        // Then borrow from right, buffers index + 1 will give right header to borrow over

    }
    ref = NULL;
    fail_find:
    fail_push:
    vstack_deinit(&bs);
    fail_stack:
    return ref;
}

void* Btree_search(struct Btree* tree, const void* data)
{
    void* result;
    struct mway_header* curr = tree->root;
    while (curr != NULL) {
        size_t index = search_node(curr, data, tree->cmp);
        if (index == (size_t) -1) {
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
 
/* =========================================================================
 * Inspection
 * ========================================================================= */

/* =========================================================================
 * Traversal
 * ========================================================================= */
 
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
    if (size == node->capacity) {
        if (index < size) {
            struct mway_entry last_entry = mway_get_entry(node, node->capacity - 1);
            // i is where we encounter first data that is bigger than new_entries data. We want to insert here, thus, shifting by 1.
            // dest is i + 1, src is i. i is zero indexed while size is not, so we use size - i gives data num from i to the last
            // instead of size - i + 1. But we need to substract 1 too, because we will discard the last data.
            memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (node->capacity - index - 1));
            mway_set_entry(node, index, &new_entry);
            return last_entry;
        } else {
            return new_entry;
        }
    }
    // This time, we dont substract size_t __n by one, since we have space
    // If i is size, we are adding to the very end and memmove shouldnt throw error, because 0 element is requested to be moved
    memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (size - index));
    mway_set_entry(node, index, &new_entry);
    (*size_ptr)++;
    return (struct mway_entry) {.data = NULL, .child = NULL};
}

static struct mway_entry split_node(struct mway_header* node, struct mway_entry last_entry, struct allocator_concept *ac)
{
    struct mway_header* new_node = mway_create(node->capacity, sizeof(struct btree_node_footer_layout), ac);
    if (!new_node) {
        LOG(LIB_LVL, CERROR, "Could not allocate new B-tree node");
        return (struct mway_entry) {.data = NULL, .child = NULL};
    }
    size_t median_index = node->capacity / 2;
    struct mway_entry median = mway_get_entry(node, median_index);
    // Move data AFTER median to new_node. Start copying from median_index + 1
    size_t copy_start_index = median_index + 1;
    size_t entries_from_node = node->capacity - copy_start_index;
    // But what will be the first child of the new_node? simply last child of the original node, two nodes pointing same node??
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

static struct mway_entry remove_data(struct mway_header *node, size_t index)
{
    size_t *size_ptr = get_node_size_ptr(node);
    assert(index < *size_ptr);
    struct mway_entry removed = mway_get_entry(node, index);
    // Shifth the buffer, deleting i, moving i + 1 into i, since i is 0 indexed, substracting -1 one more time from (size - i) to get the count of remaining
    // 0 elements are handled naturally, causing memmove to take 0 as the third parameter
    memmove(mway_get_entry_addr(node, index), mway_get_entry_addr(node, index + 1), sizeof(struct mway_entry) * (*size_ptr - (index + 1)));
    (*size_ptr)--;
    return removed;
}
/*if (get_node_size(donor) <= (((donor->capacity + 1) + 1) / 2 - 1))
        return 1; */
static void borrow_from_left(struct mway_header *donor, struct mway_entry *entry)
{
    size_t target_size = get_node_size(entry->child);
    memmove(mway_get_entry_addr(entry->child, 1), mway_get_entry_addr(entry->child, 0), sizeof(struct mway_entry) * target_size);
    // Set first entry of 'starving' node
    struct mway_entry *dest = mway_get_entry_addr(entry->child, 0);
    dest->data = entry->data;
    dest->child = get_node_first_child(entry->child);
    // Get last donor
    size_t donor_size = get_node_size(donor);
    struct mway_entry donor_last = mway_get_entry(donor, donor_size - 1);
    // Set parents data
    entry->data = donor_last.data;
    // Set first child of 'starving' node to donors child, since it will be strictly less
    *get_node_first_child_ptr(entry->child) = donor_last.child;
    // Update sizes
    *get_node_size_ptr(donor) = donor_size - 1;
    *get_node_size_ptr(entry->child) = target_size + 1;
}

static void borrow_from_right(struct mway_header *starving, struct mway_entry *entry)
{
    struct mway_header *donor = entry->child;
    size_t starving_size = get_node_size(starving);
    // 1. Append Parent's data to the end of starving node
    // No memmove needed for starving node, we just append
    struct mway_entry *dest = mway_get_entry_addr(starving, starving_size);
    dest->data = entry->data;
    // The new child of starving is the *leftmost* child of the donor
    // (Everything in that subtree is less than the donor's first key, but greater than parent key)
    dest->child = get_node_first_child(donor);
    // 2. Move Donor's first data up to Parent
    struct mway_entry *donor_first = mway_get_entry_addr(donor, 0);
    entry->data = donor_first->data;
    // 3. Fix Donor's "First Child" pointer
    // The child associated with the key we just moved up (donor_first->child) 
    // becomes the new leftmost boundary for the donor.
    *get_node_first_child_ptr(donor) = donor_first->child;
    // 4. Shift Donor entries left to cover the gap
    // We overwrite index 0 with index 1, etc.
    size_t donor_size = get_node_size(donor);
    memmove(mway_get_entry_addr(donor, 0), mway_get_entry_addr(donor, 1), sizeof(struct mway_entry) * (donor_size - 1));
    // 5. Update sizes
    *get_node_size_ptr(starving) = starving_size + 1;
    *get_node_size_ptr(donor) = donor_size - 1;
}

static size_t search_node(struct mway_header* node, const void* data, int (*cmp) (const void* key, const void* data))
{
    size_t size = get_node_size(node);
    for (size_t i = 0; i < size; i++) {
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
    for (size_t i = 0; i < size; i++) {
        handler(mway_get_data(root, i), context);
        walk_helper(mway_get_child(root, i), context, handler);
    }
}

static struct mway_header* get_node_first_child(const struct mway_header *node)
{
    const struct btree_node_footer_layout *footer = (const struct btree_node_footer_layout *) mway_get_footer_const(node);
    return footer->first_child;
}

static size_t get_node_size(const struct mway_header *node)
{
    const struct btree_node_footer_layout *footer = (const struct btree_node_footer_layout *) mway_get_footer_const(node);
    return footer->current_size;
}

static struct mway_header** get_node_first_child_ptr(struct mway_header* node)
{
    struct btree_node_footer_layout *footer = (struct btree_node_footer_layout *) mway_get_footer_const(node);
    return &footer->first_child;
}

static size_t* get_node_size_ptr(struct mway_header* node)
{
    struct btree_node_footer_layout *footer = (struct btree_node_footer_layout *) mway_get_footer_const(node);
    return &footer->current_size;
}