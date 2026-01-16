#include <ds/trees/trie.h>
#include <assert.h>

static enum trees_status trie_get_entry(struct trie *tr, const char *key, struct mway_entry **result);
static void trie_deinit_helper(struct mway_header *header, struct object_concept *oc, struct allocator_concept *ac);
// Returning 1 means continue, 0 means stop.
static int trie_traverse_helper(struct mway_header *header, char *buffer, size_t depth, void *context, trie_visit_cb cb, trie_unmap_cb unmap);
static int trie_node_is_empty(struct mway_header *header);

/* =========================================================================
 * Initialie & Deinitialize
 * ========================================================================= */

void trie_init(struct trie *tr, struct allocator_concept *ac, size_t alphabet_size, trie_map_cb mapper, trie_unmap_cb unmapper)
{
    assert(tr != NULL && ac != NULL && mapper != NULL);
    tr->root.child = NULL;
    tr->root.data = NULL;
    tr->ac = *ac;
    tr->alphabet_size = alphabet_size;
    tr->count = 0;
    tr->mapper = mapper;
    tr->unmapper = unmapper;
}

void trie_deinit(struct trie *tr, struct object_concept *oc)
{
    assert(tr != NULL);
    trie_deinit_helper(tr->root.child, oc, &tr->ac);
    tr->root.child = NULL;
    if (oc && oc->deinit)
        oc->deinit(tr->root.data);
    tr->root.data = NULL;
}

/* =========================================================================
 * Dictionary Operations
 * ========================================================================= */

void *trie_put(struct trie *tr, const char *key, void *value)
{
    assert(tr != NULL && key != NULL && value != NULL);
    struct mway_entry *curr = &tr->root;
    while (*key) {
        if (curr->child == NULL) {
            struct mway_header *new_node = mway_create(tr->alphabet_size, 0, &tr->ac);
            if (!new_node) {
                LOG(LIB_LVL, CERROR, "Allocation failure");
                return NULL;
            }
            curr->child = new_node;
        }
        size_t index = tr->mapper(*key);
        if (index >= tr->alphabet_size) {
            LOG(LIB_LVL, CERROR, "Unknown input received");
            return NULL;
        }
        curr = mway_get_entry_addr(curr->child, index);
        key++;
    }
    void *old_val = curr->data;
    if (old_val == NULL)
        tr->count++;
    curr->data = value;
    return old_val;
}

enum trees_status trie_get(struct trie *tr, const char *key, void **value)
{
    assert(tr != NULL && key != NULL);
    struct mway_entry *result;
    enum trees_status stat = trie_get_entry(tr, key, &result);
    if (stat == TREES_OK) {
        if (result->data != NULL) {
            *value = result->data;
        } else {
            return TREES_NOT_FOUND;
        }
    }
    return stat;
}

enum trees_status trie_remove(struct trie *tr, const char *key, void **removed)
{
    assert(tr != NULL && key != NULL);
    struct mway_entry *result;
    enum trees_status stat = trie_get_entry(tr, key, &result);
    if (stat == TREES_OK) {
        if (result->data != NULL) {
            *removed = result->data;
            result->data = NULL;
            tr->count--;
        } else {
            return TREES_NOT_FOUND;
        }
    }
    return stat;
}

int trie_contains(struct trie *tr, const char *key)
{
    void *dummy;
    return trie_get(tr, key, &dummy) == TREES_OK;
}

/* =========================================================================
 * Advanced Operations
 * ========================================================================= */

void trie_prefix_iterate(struct trie *tr, const char *prefix, void *context, trie_visit_cb cb)
{
    assert(tr != NULL && prefix != NULL && cb != NULL);
    size_t prefix_len = strlen(prefix);
    if (prefix_len >= TRIE_STACK_DEPTH - 1) {
        LOG(LIB_LVL, CERROR, "Prefix exceeds stack depth limit");
        return;
    }
    struct mway_entry *result;
    enum trees_status stat = trie_get_entry(tr, prefix, &result);
    if (stat != TREES_OK)
        return;
    char buffer[TRIE_STACK_DEPTH];
    strncpy(buffer, prefix, TRIE_STACK_DEPTH); 
    buffer[TRIE_STACK_DEPTH - 1] = '\0';
    if (result->data) {
        if (!cb(buffer, result->data, context))
            return;
    }
    if (result->child)
        trie_traverse_helper(result->child, buffer, prefix_len, context, cb, tr->unmapper);
}

size_t trie_longest_prefix(struct trie *tr, const char *key)
{
    assert(tr != NULL && key != NULL);
    struct mway_entry *curr = &tr->root;
    size_t current_len = 0;
    size_t longest_match_len = 0;
    // Check if empty string key exists
    if (curr->data) {
        longest_match_len = 0; // It's a match of length 0
    }
    while (*key) {
        // 1. Can we go deeper?
        if (curr->child == NULL)
            break;
        size_t index = tr->mapper(*key);
        if (index >= tr->alphabet_size)
            break;
        // 2. Advance
        curr = mway_get_entry_addr(curr->child, index);
        key++;
        current_len++;
        // 3. Is this node a registered key?
        if (curr->data != NULL)
            longest_match_len = current_len;
    }
    return longest_match_len;
}

// *** Helper Functions *** //

static enum trees_status trie_get_entry(struct trie *tr, const char *key, struct mway_entry **result)
{
    struct mway_entry *curr = &tr->root;
    while (*key) {
        if (curr->child == NULL)
            return TREES_NOT_FOUND;
        size_t index = tr->mapper(*key);
        if (index >= tr->alphabet_size)
            return TREES_UNKNOWN_INPUT;
        curr = mway_get_entry_addr(curr->child, index);
        key++;
    }
    *result = curr;
    return TREES_OK;
}

static void trie_deinit_helper(struct mway_header *header, struct object_concept *oc, struct allocator_concept *ac)
{
    if (!header)
        return;
    for (size_t i = 0; i < header->capacity; i++) {
        trie_deinit_helper(mway_get_child(header, i), oc, ac);
        if (oc && oc->deinit) {
            void* data = mway_get_data(header, i);
            if (data)
                oc->deinit(data);
        }
    }
    ac->free(ac->allocator, header);
}

static int trie_traverse_helper(struct mway_header *header, char *buffer, size_t depth, void *context, trie_visit_cb cb, trie_unmap_cb unmap)
{
    for (size_t i = 0; i < header->capacity; i++) {
        struct mway_entry *entry = mway_get_entry_addr(header, i);
        unsigned char c = unmap(i);
        buffer[depth] = (char)c;
        buffer[depth + 1] = '\0';
        if (entry->data) {
            if (!cb(buffer, entry->data, context))
                return 0; 
        }
        if (entry->child) {
            if (!trie_traverse_helper(entry->child, buffer, depth + 1, context, cb, unmap))
                return 0;
        }
    }
    return 1;
}

static int trie_node_is_empty(struct mway_header *header)
{
    if (!header) return 1;
    for (size_t i = 0; i < header->capacity; i++) {
        struct mway_entry *e = mway_get_entry_addr(header, i);
        // If any entry has a child OR data, this node is still in use
        if (e->child != NULL || e->data != NULL) return 0;
    }
    return 1;
}