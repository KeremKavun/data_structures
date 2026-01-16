#include <ds/trees/Btree.h>
#include <stdlib.h>
#include <assert.h>

#define BTREE_MAX_HEIGHT 64

/*
    Btree nodes are classic mway node concept introduced in mwaytree.h + first node + size_t size to track the count of data
    allocated in the footer area. Here how it look like:

    struct mway_header;
    struct mway_entry[N];
    struct mway_header *first_child;
    size_t size;
*/

/*
    Assuming no duplicate is passed for now.
*/

struct btree_buffer {
    struct mway_header      *node;
    size_t                  index;
};

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
static struct mway_entry insert_data(struct mway_header *node, struct mway_entry new_entry, size_t index);

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
static struct mway_entry split_node(struct mway_header *node, struct mway_entry last_entry, struct allocator_concept *ac);

/**
 * @brief Just save the entry to be removed and then shift.
 * 
 * @param[in, out] node Node to remove the entry.
 * @param[in] index Index of entry to be removed.
 * 
 * @return Removed entry from the B-tree.
 */
static void *remove_entry(struct mway_header *node, size_t index);

/**
 * @brief Performs a "Right Rotation" to fix an underflow in the right sibling.
 * * ### Mental Model
 * Think of the Parent Key as a pivot. The Donor (Left Sibling) pushes its 
 * largest value UP to the Parent, which forces the Parent's existing separator 
 * DOWN into the Starving (Right Sibling) node.
 * * ### Data Flow
 * 1. **Shift Starving:** Make room at index 0.
 * 2. **Parent -> Starving:** The separator key moves down to `starving[0]`.
 * 3. **Donor -> Parent:** The largest key from Donor moves up to replace the parent separator.
 * * ### Pointer Surgery
 * The critical pointer movement involves the Donor's rightmost child:
 * - It represents values > Donor's max key but < Parent's old key.
 * - It becomes the **First Child** of the Starving node.
 * * @param[in,out] donor The left sibling that has extra keys to spare.
 * @param[in,out] entry The parent entry pointing to the **starving** node (entry->child).
 */
static void borrow_from_left(struct mway_header *donor, struct mway_entry *entry);

/**
 * @brief Performs a "Left Rotation" to fix an underflow in the left sibling.
 * * ### Mental Model
 * The Donor (Right Sibling) pushes its smallest value UP to the Parent, 
 * bumping the Parent's existing separator DOWN into the Starving (Left Sibling) node.
 * * ### Data Flow
 * 1. **Parent -> Starving:** The separator key moves down to the *end* of the Starving node.
 * 2. **Donor -> Parent:** The smallest key from Donor moves up to replace the parent separator.
 * 3. **Shift Donor:** The Donor shifts remaining entries left to cover the gap.
 * * ### Pointer Surgery
 * The critical pointer movement involves the Donor's "First Child" (stored in footer):
 * - It represents values < Donor's min key but > Parent's old key.
 * - It becomes the child associated with the new entry appended to the Starving node.
 * * @param[in,out] starving The left sibling that is underflowed.
 * @param[in,out] entry    The parent entry pointing to the **donor** node (entry->child).
 */
static void borrow_from_right(struct mway_header *starving, struct mway_entry *entry);

/**
 * @brief Merges two sibling nodes and the parent separator into a single node.
 * * ### Mental Model (The "Sandwich")
 * When neither sibling can lend a key, we must merge. The Left Node absorbs 
 * both the Parent Separator and the Right Node.
 * `[Left Data] + [Parent Separator] + [Right Data] => [New Left Node]`
 * * ### Data Flow
 * 1. **Parent -> Left:** The separator key at `index` is appended to the Left Node.
 * 2. **Right -> Left:** All entries from the Right Node are appended to the Left Node.
 * 3. **Cleanup:** The Right Node is freed, and the separator is removed from the Parent.
 * * ### Pointer Surgery
 * - The Right Node's "First Child" becomes the child pointer for the 
 * separator key that moved down from the parent.
 * - The parent node loses one child pointer (the one pointing to the Right Node).
 * * @param[in,out] parent_node The node containing the separator and child pointers.
 * @param[in]     index       The index of the separator in the parent node.
 * - Left Sibling is at `child(index-1)` (or first_child if index==0).
 * - Right Sibling is at `child(index)`.
 * @param[in]     ac          Allocator concept used to free the Right Node.
 */
static void merge_starvings(struct mway_header *parent_node, size_t index, struct allocator_concept *ac);

/**
 * @brief Searches for a specific data item within a node.
 * * Performs a binary search ($O(\log N)$) for the input @p data.
 * * @param[in] node The node to search.
 * @param[in] data The key/data to search for.
 * @param[in] cmp  Comparison function (returns 0 if key == data, >0 if key > data).
 * @param[out] it  Output parameter with dual behavior:
 * - If found: Contains the index of the data in the node.
 * - If NOT found: Contains the index of the child pointer to follow.
 * ((size_t)-1 for the first child, 0..size-1 for subsequent children).
 * * @return A pointer to the data item if found; otherwise NULL.
 */
static void *search_data(struct mway_header *node, const void *data, int (*cmp) (const void *key, const void *data), size_t *it);

/**
 * @brief Recursive helper for in-order traversal of the tree.
 * Visits the "first child" (footer), then iterates through entries, visiting 
 * data and then the subsequent child.
 * 
 * @param[in] root The current node.
 * @param[in] context User-provided context passed to the handler.
 * @param[in] handler Callback function to process each data item.
 */
static void walk_helper(struct mway_header *root, void *context, void (*handler) (void *item, void *context));

/** 
 * @brief Gets the "leftmost" child pointer stored in the node's footer.
 * @note This pointer handles the range of values strictly less than entry[0].
 */
static struct mway_header *get_node_first_child(const struct mway_header *node);

/** 
 * @brief Gets the current number of active entries (size) from the footer.
 * @note This is distinct from capacity. Capacity is fixed; size varies.
 */
static size_t get_node_size(const struct mway_header *node);

/** @return Mutable pointer to the "leftmost" child field in the footer. */
static struct mway_header** get_node_first_child_ptr(struct mway_header *node);

/** @return Mutable pointer to the size field in the footer. */
static size_t* get_node_size_ptr(struct mway_header *node);

static void destroy_helper(struct mway_header* header, struct object_concept *oc, struct allocator_concept *ac);

/* =========================================================================
 * Initialize & Deinitialize
 * ========================================================================= */

int Btree_init(struct Btree *tree, size_t order, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac)
{
    assert(tree != NULL && cmp != NULL && ac != NULL);
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    struct mway_header *root = mway_create(order - 1, sizeof(struct btree_node_footer_layout), ac);
    if (!root) {
        LOG(LIB_LVL, CERROR, "Could not allocate B-tree root");
        return 1;
    }
    tree->root = root;
    tree->ac = *ac;
    tree->cmp = cmp;
    tree->size = 0;
    return 0;
}

void Btree_deinit(struct Btree *tree, struct object_concept *oc)
{
    destroy_helper(tree->root, oc, &tree->ac);
    tree->root = NULL;
}

/* =========================================================================
 * Operations
 * ========================================================================= */

enum trees_status Btree_add(struct Btree *tree, void *new_data)
{
    assert(tree != NULL && new_data != NULL);
    enum trees_status status = TREES_SYSTEM_ERROR;
    struct btree_buffer stack[BTREE_MAX_HEIGHT]; 
    int stack_ptr = 0;
    struct mway_header *curr = tree->root;
    size_t index;
    while (curr != NULL) {
        if (stack_ptr >= BTREE_MAX_HEIGHT)
            goto fail_push;
        // Save index in search_data into buffer
        void *data = search_data(curr, new_data, tree->cmp, &index);
        if (data != NULL) {
            LOG(LIB_LVL, CERROR, "Duplicate inputs arent supported yet");
            status = TREES_DUPLICATE_KEY;
            goto fail_push;
        }
        stack[stack_ptr++] = (struct btree_buffer) { .node = curr, .index = index };
        // Update curr. If index is (size_t) -1, proceed with first child
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    struct mway_entry curr_entry = {.data = new_data, .child = NULL};
    // Keep going until either root or no overflow or split node raises errors by setting curr_entry to NULLs
    while (stack_ptr > 0 && (curr_entry.data != NULL || curr_entry.child != NULL)) {
        struct btree_buffer buffer = stack[--stack_ptr];
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
        struct mway_header *new_root = mway_create(tree->root->capacity, sizeof(struct btree_node_footer_layout), &tree->ac);
        if (!new_root) {
            LOG(LIB_LVL, CERROR, "Could not allocate new B-tree root");
            goto fail_root;
        }
        struct mway_header *tmp = tree->root;
        tree->root = new_root;
        // Set first child to previous root, which was remained in the left after the last split
        *get_node_first_child_ptr(tree->root) = tmp;
        mway_set_entry(tree->root, 0, &curr_entry);
        *get_node_size_ptr(tree->root) = 1;
        // Result should be a root with first child old root and median is first entry
    }
    tree->size++;
    status = TREES_OK;
    fail_root:
    fail_split:
    fail_push:
    return status;
}

static int is_underflowed(struct mway_header *node)
{
    size_t m = node->capacity + 1;
    size_t min_entries = (m + 1) / 2 - 1; // Standard integer ceiling for (m/2) - 1
    return get_node_size(node) < min_entries;
}

void *Btree_remove(struct Btree *tree, void *data)
{
    assert(tree != NULL && data != NULL);
    void *result = NULL; // Renamed to avoid confusion
    struct btree_buffer stack[BTREE_MAX_HEIGHT]; 
    int stack_ptr = 0;
    struct mway_header *curr = tree->root;
    size_t index;
    // Store the path
    while (curr != NULL) {
        if (stack_ptr >= BTREE_MAX_HEIGHT)
            goto fail;
        // Note: search_data returns ptr if found, updates index to child index if not
        result = search_data(curr, data, tree->cmp, &index);
        if (result) {
            // Note index cannot be -1, since we found a data in leaf, exit
            if (get_node_first_child(curr) == NULL)
                break; 
            // Save current node and index to later set the data to the predecessor
            struct mway_header *target_node = curr;
            size_t target_idx = index;
            // Push current node to stack before descending to left child
            stack[stack_ptr++] = (struct btree_buffer) { .node = curr, .index = index - 1 };
            // If index is 0, we go to first_child. Otherwise we go to child[index-1].
            curr = (index == 0) ? get_node_first_child(curr) : mway_get_child(curr, index - 1);
            while (get_node_first_child(curr) != NULL) {
                // Note size cannot be -1 again, since the node cannot be empty here, handled elsewhere
                index = get_node_size(curr) - 1;
                stack[stack_ptr++] = (struct btree_buffer) { .node = curr, .index = index };
                curr = mway_get_child(curr, index);
            }
            // Set index to use in remove_entry function just after the loop
            index = get_node_size(curr) - 1;
            mway_set_data(target_node, target_idx, mway_get_data(curr, index));
            break;
        }
        // Not found yet, continue search
        stack[stack_ptr++] = (struct btree_buffer) { .node = curr, .index = index };
        curr = (index != (size_t) -1) ? mway_get_child(curr, index) : get_node_first_child(curr);
    }
    if (!result)
        goto fail;
    remove_entry(curr, index);
    // curr is now the leaf node where the physical deletion happened
    while (stack_ptr > 0 && is_underflowed(curr)) {
        struct btree_buffer parent = stack[--stack_ptr];
        // First try to borrow from left
        // Firstly, parent shouldnt be at the index -1
        if (parent.index != (size_t) -1) {
            struct mway_header *left_donor = (parent.index != 0) ? mway_get_child(parent.node, parent.index - 1) : get_node_first_child(parent.node);
            // Secondly and finally, left_donors size should be greater than ceil(m/2) - 1
            if (get_node_size(left_donor) > ((Btree_order(tree) + 1) / 2) - 1) {
                borrow_from_left(left_donor, mway_get_entry_addr(parent.node, parent.index));
                // No node has been underflowed, exiting
                break;
            }
        }
        // If couldnt, try to borrow from right
        // Firstly, parent shouldnt be at the index get_node_size(parent.node) - 1
        if (parent.index != get_node_size(parent.node) - 1) {
            struct mway_header *left_starving = (parent.index != (size_t) -1) ? mway_get_child(parent.node, parent.index) : get_node_first_child(parent.node);
            // Note: If parent.index is -1, parent.index + 1 is 0, which is valid.
            if (get_node_size(mway_get_child(parent.node, parent.index + 1)) > ((Btree_order(tree) + 1) / 2) - 1) {
                // If parent.index is -1, we borrow using entry[0] as pivot
                size_t entry_idx = (parent.index == (size_t)-1) ? 0 : parent.index + 1;
                borrow_from_right(left_starving, mway_get_entry_addr(parent.node, entry_idx));
                break;
            }
        }
        // Else, merge the nodes
        size_t merge_idx = (parent.index == (size_t) -1) ? 0 : parent.index;
        merge_starvings(parent.node, merge_idx, &tree->ac);
        curr = parent.node;
    }
    // Handle Root Underflow
    if (get_node_size(tree->root) == 0) {
        struct mway_header *old_root = tree->root;
        // If root is empty but has a child (result of merge), make child the new root
        if (get_node_first_child(old_root) != NULL) {
            tree->root = get_node_first_child(old_root);
            tree->ac.free(tree->ac.allocator, old_root);
        }
    }
    tree->size--;
fail:
    return result;
}

void *Btree_search(struct Btree *tree, const void *data)
{
    assert(tree != NULL);
    struct mway_header *curr = tree->root;
    size_t i;
    while (curr != NULL) {
        void *cand = search_data(curr, data, tree->cmp, &i);
        if (cand != NULL)
            return cand;
        curr = (i != (size_t) -1) ? mway_get_child(curr, i) : get_node_first_child(curr);
    }
    return NULL;
}
 
/* =========================================================================
 * Inspection
 * ========================================================================= */

/* =========================================================================
 * Traversal
 * ========================================================================= */
 
void Btree_walk(struct Btree *tree, void *context, void (*handler) (void *data, void *context))
{
    assert(tree != NULL && handler != NULL);
    walk_helper(tree->root, context, handler);
}

// *** Helper functions *** //

static struct mway_entry insert_data(struct mway_header *node, struct mway_entry new_entry, size_t index)
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
            if (index < node->capacity - 1)
                memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (node->capacity - index - 1));
            mway_set_entry(node, index, &new_entry);
            return last_entry;
        } else {
            return new_entry;
        }
    }
    // This time, we dont substract size_t __n by one, since we have space
    // If i is size, we are adding to the very end and memmove shouldnt throw error, because 0 element is requested to be moved
    if (index < size)
        memmove(mway_get_entry_addr(node, index + 1), mway_get_entry_addr(node, index), sizeof(struct mway_entry) * (size - index));
    mway_set_entry(node, index, &new_entry);
    (*size_ptr)++;
    return (struct mway_entry) {.data = NULL, .child = NULL};
}

static struct mway_entry split_node(struct mway_header *node, struct mway_entry last_entry, struct allocator_concept *ac)
{
    struct mway_header *new_node = mway_create(node->capacity, sizeof(struct btree_node_footer_layout), ac);
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

static void *remove_entry(struct mway_header *node, size_t index)
{
    size_t *size_ptr = get_node_size_ptr(node);
    assert(index < *size_ptr);
    // 1. Get the data to return
    void *removed = mway_get_data(node, index);
    // 2. Calculate remaining items
    size_t remaining = *size_ptr - (index + 1);
    // 3. CRITICAL FIX: Only call memmove if there are items to shift.
    // If we are removing the last item (remaining == 0), index + 1 equals size.
    // If size == capacity, calculating address of (index+1) triggers the assertion.
    if (remaining > 0)
        memmove(mway_get_entry_addr(node, index), mway_get_entry_addr(node, index + 1), sizeof(struct mway_entry) * remaining);
    (*size_ptr)--;
    return removed;
}

static void borrow_from_left(struct mway_header *donor, struct mway_entry *entry)
{
    struct mway_header *starving = entry->child;
    size_t starving_size = get_node_size(starving);
    memmove(mway_get_entry_addr(starving, 1), mway_get_entry_addr(starving, 0), sizeof(struct mway_entry) * starving_size);
    // Set first entry of 'starving' node
    struct mway_entry *dest = mway_get_entry_addr(starving, 0);
    dest->data = entry->data;
    // Just shifted array, so we must consider first child too.
    dest->child = get_node_first_child(starving);
    // Get last donor
    size_t donor_size = get_node_size(donor);
    struct mway_entry donor_last = mway_get_entry(donor, donor_size - 1);
    // Set parents data
    entry->data = donor_last.data;
    // Last child of donor stores objects lesser than parent, greater than donors last child.
    // Since we move parent to starving and donor last to parent, we set first child of the starving to donor_lasts child.
    *get_node_first_child_ptr(starving) = donor_last.child;
    // Update sizes
    *get_node_size_ptr(donor) = donor_size - 1;
    *get_node_size_ptr(starving) = starving_size + 1;
}

static void borrow_from_right(struct mway_header *starving, struct mway_entry *entry)
{
    struct mway_header *donor = entry->child;
    size_t starving_size = get_node_size(starving);
    // Set last entry of 'starving' node
    struct mway_entry *dest = mway_get_entry_addr(starving, starving_size);
    dest->data = entry->data;
    // First child of donor stores objects greater than parent, lesser than donors first child.
    // Since we move parent to starving and donor first to parent, we set dest child to first child of the donor.
    dest->child = get_node_first_child(donor);
    // Get first donor
    size_t donor_size = get_node_size(donor);
    struct mway_entry *donor_first = mway_get_entry_addr(donor, 0);
    // Set parents data
    entry->data = donor_first->data;
    // Will shift array, so we must consider first child too.
    *get_node_first_child_ptr(donor) = donor_first->child;
    memmove(mway_get_entry_addr(donor, 0), mway_get_entry_addr(donor, 1), sizeof(struct mway_entry) * (donor_size - 1));
    // Update sizes
    *get_node_size_ptr(starving) = starving_size + 1;
    *get_node_size_ptr(donor) = donor_size - 1;
}

static void merge_starvings(struct mway_header *parent_node, size_t index, struct allocator_concept *ac)
{
    struct mway_header *left_starving = (index != 0) ? mway_get_child(parent_node, index - 1) : get_node_first_child(parent_node);
    size_t left_starving_size = get_node_size(left_starving);
    struct mway_header *right_starving = mway_get_child(parent_node, index);
    size_t right_starving_size = get_node_size(right_starving);
    // Set last entry of 'left starving' node to the parent
    struct mway_entry *dest = mway_get_entry_addr(left_starving, left_starving_size);
    dest->data = mway_get_data(parent_node, index);
    // First child of right_starving stores objects greater than parent, lesser than right_starving first child.
    // Since we move parent to left_starving and copy right_starving, we set dest child to first child of the right_starving.
    dest->child = get_node_first_child(right_starving);
    // Copy right starving nodes entries into the left starving, notice left_starving_size will be passed one more, since we appended the parent.
    memcpy(mway_get_entry_addr(left_starving, left_starving_size + 1), mway_get_entry_addr(right_starving, 0), sizeof(struct mway_entry) * right_starving_size);
    // Update merged nodes (left_starving) size
    *get_node_size_ptr(left_starving) = left_starving_size + right_starving_size + 1;
    // Safely delete parent entries child, right_starving. One previous child ptr will be pointing to the new merged node.
    ac->free(ac->allocator, right_starving);
    // Now remove parent entry from the parent node
    remove_entry(parent_node, index);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// TODO: In the future, you might implement generic binary search an arrays and use here
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

static void *search_data(struct mway_header *node, const void *data, int (*cmp) (const void *key, const void *data), size_t *it)
{
    size_t size = get_node_size(node);
    // CRITICAL FIX: Handle empty node to prevent underflow on (size - 1)
    if (size == 0) {
        *it = (size_t)-1;
        return NULL;
    }
    size_t low = 0;
    size_t high = size - 1;
    while (low <= high) {
        size_t mid = low + (high - low) / 2;
        void *mid_data = mway_get_data(node, mid);
        int result = cmp(mid_data, data);
        if (result == 0) {
            *it = mid; // FUTURE-PROOFING: Store index of found item
            return mid_data;
        } else if (result > 0) {
            if (mid == 0) {
                low = 0; 
                break;
            }
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    // Logic verification:
    // If Data < K0: low=0 -> it = -1 (First Child)
    // If K0 < Data < K1: low=1 -> it = 0 (Child right of K0)
    // If Data > Klast: low=size -> it = size-1 (Child right of Klast)
    *it = low - 1;
    return NULL;
}

static void walk_helper(struct mway_header *root, void *context, void (*handler) (void *item, void *context))
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

static struct mway_header *get_node_first_child(const struct mway_header *node)
{
    const struct btree_node_footer_layout *footer = (const struct btree_node_footer_layout *) mway_get_footer_const(node);
    return footer->first_child;
}

static size_t get_node_size(const struct mway_header *node)
{
    const struct btree_node_footer_layout *footer = (const struct btree_node_footer_layout *) mway_get_footer_const(node);
    return footer->current_size;
}

static struct mway_header** get_node_first_child_ptr(struct mway_header *node)
{
    struct btree_node_footer_layout *footer = (struct btree_node_footer_layout *) mway_get_footer_const(node);
    return &footer->first_child;
}

static size_t* get_node_size_ptr(struct mway_header *node)
{
    struct btree_node_footer_layout *footer = (struct btree_node_footer_layout *) mway_get_footer_const(node);
    return &footer->current_size;
}

static void destroy_helper(struct mway_header* header, struct object_concept *oc, struct allocator_concept *ac)
{
    if (!header)
        return;
    destroy_helper(get_node_first_child(header), oc, ac);
    for (size_t i = 0; i < get_node_size(header); i++) {
        destroy_helper(mway_get_child(header, i), oc, ac);
    }
    if (oc && oc->deinit) {
        for (size_t i = 0; i < get_node_size(header); i++) {
            void* data = mway_get_data(header, i);
            if (data)
                oc->deinit(data);
        }
    }
    ac->free(ac->allocator, header);
}