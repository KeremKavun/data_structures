#include "../../../prime/include/primes.h"
#include <ds/hashs/hash_table.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BASE_PRIME 53
#define UP_LOAD_RATIO 0.7
#define DOWN_LOAD_RATIO 0.1
#define FACTOR_UP 2
#define FACTOR_DOWN 0.5

struct ht_item {
   void*        key;
   void*        value;
};

struct hash_table {
   struct ht_item*          items;
   size_t                   capacity;
   size_t                   size;
   struct hash_concept      hc;
};

// ht_item helpers

// Init item, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure
static void init_item(struct ht_item* item, void* key, void* value);
static void set_key(struct ht_item* item, void* key);
static void set_value(struct ht_item* item, void* value);
static const void* get_key(struct ht_item* item);
static const void* get_value(struct ht_item* item);
// Free ht_item key value pair and set it to zero
static void mark_item(struct ht_item* item);
// Returns 1 if the item is null (never assigned something), 0 otherwise
static int is_null(struct ht_item* item);
// Returns 1 if the item was deleted before, 0 otherwise
static int is_deleted(struct ht_item* item);

// hash_table helpers

// hash_table_init helper. Inits size and memory realted attributes.
// Leaves object's state the same as before the function call in case of failure
static int init_size_ht(struct hash_table* ht, size_t capacity);
// Resize hash_table, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize(struct hash_table* ht, float factor);
// Increase hash_table size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize_up(struct hash_table* ht, float load);
// Decrease hash_table size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize_down(struct hash_table* ht, float load);

static void* deleted;

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct hash_table* hash_table_create(struct hash_concept *hc)
{
    assert(hc != NULL);
    struct hash_table* ht = malloc(sizeof(*ht));
    if (ht == NULL) {
        LOG(LIB_LVL, CERROR, "malloc failed");
        return NULL;
    }
    if (init_size_ht(ht, BASE_PRIME) != 0) {
        LOG(LIB_LVL, CERROR, "init_size_ht failed");
        free(ht);
        return NULL;
    }
    ht->hc = *hc;
    return ht;
}

void hash_table_destroy(struct hash_table* ht, struct object_concept *oc)
{
    assert(ht != NULL);
    if (oc != NULL && oc->deinit != NULL) {
        for (size_t i = 0; i < ht->capacity; i++) {
            if (!is_null(&ht->items[i]) && !is_deleted(&ht->items[i])) {
                oc->deinit(ht->items[i].key);
                oc->deinit(ht->items[i].value);
            }
        }
    }
    free(ht->items);
    free(ht);
}

/* =========================================================================
 * Insertion & Removal
 * ========================================================================= */

int hash_table_insert(struct hash_table* ht, void* key, void* value)
{
    assert(ht != NULL && key != NULL && value != NULL);
    if (resize_up(ht, (float) ht->size / ht->capacity) != 0) {
        LOG(LIB_LVL, CERROR, "Could not resize the hash_table up");
        return 1;
    }
    int attempts = 0;
    int index = ht->hc.hash(key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    struct ht_item* first_deleted = NULL;
    while (!is_null(curr_item)) {
        if (is_deleted(curr_item)) {
            if (!first_deleted)
                first_deleted = curr_item;
        } else if (ht->hc.cmp_key(get_key(curr_item), key) == 0) {
            set_value(curr_item, value);
            return 0;
        }
        attempts++;
        index = ht->hc.hash(key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }
    init_item((first_deleted) ? first_deleted : curr_item, key, value);
    ht->size++;
    return 0;
}

int hash_table_remove(struct hash_table* ht, const void* key)
{
    assert(ht != NULL && key != NULL);
    int attempts = 0;
    int index = ht->hc.hash(key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    while (!is_null(curr_item)) {
        if (!is_deleted(curr_item) && ht->hc.cmp_key(get_key(curr_item), key) == 0) {
            // save current because possible resize will invalidate curr_item
            struct ht_item copy_curr = *curr_item;
            mark_item(curr_item);
            ht->size--;
            if (resize_down(ht, (float) ht->size / ht->capacity) != 0) {
                LOG(LIB_LVL, CERROR, "Could not resize the hash_table down");
                *curr_item = copy_curr;
                return 1;
            }
            return 0;
        }
        attempts++;
        index = ht->hc.hash(key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }
    LOG(LIB_LVL, CERROR, "The key to be deleted couldnt be found");
    return 1;
}

/* =========================================================================
 * Properties
 * ========================================================================= */

size_t hash_table_size(const struct hash_table* ht)
{
    return ht->size;
}

size_t hash_table_capacity(const struct hash_table* ht)
{
    return ht->capacity;
}

/* =========================================================================
 * Search
 * ========================================================================= */

void* hash_table_search(struct hash_table* ht, const void* key)
{
    assert(ht != NULL && key != NULL);
    int attempts = 0;
    int index = ht->hc.hash(key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    while (!is_null(curr_item)) {
        if (!is_deleted(curr_item) && ht->hc.cmp_key(get_key(curr_item), key) == 0)
            return curr_item->value;
        attempts++;
        index = ht->hc.hash(key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }
    return NULL; // Key not found
}

/* =========================================================================
 * Iteration
 * ========================================================================= */

void hash_table_walk(struct hash_table* ht, void* context, void (*exec) (void* key, void* value, void* context))
{
    assert(ht != NULL && exec != NULL);
    for (size_t i = 0; i < ht->capacity; i++) {
        if (!is_null(&ht->items[i]) && !is_deleted(&ht->items[i]))
            exec(ht->items[i].key, ht->items[i].value, context);
    }
}

// *** Helper functions *** //

static int init_size_ht(struct hash_table* ht, size_t capacity)
{
    struct ht_item* _items = calloc(capacity, sizeof(struct ht_item));
    if (!_items) {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    ht->items = _items;
    ht->capacity = capacity;
    ht->size = 0;
    return 0;
}

static void init_item(struct ht_item* item, void* key, void* value)
{
    set_key(item, key);
    set_value(item, value);
}

static inline void set_key(struct ht_item* item, void* key)
{
    item->key = key;
}

static inline void set_value(struct ht_item* item, void* value)
{
    item->value = value;
}

static inline const void* get_key(struct ht_item* item)
{
    return item->key;
}

static inline const void* get_value(struct ht_item* item)
{
    return item->value;
}

static void mark_item(struct ht_item* item)
{
    set_key(item, &deleted);
    set_value(item, &deleted);
}

static int is_null(struct ht_item* item)
{
    static struct ht_item null_item;
    return !memcmp(&null_item, item, sizeof(struct ht_item));
}

static inline int is_deleted(struct ht_item* item)
{
    return (get_key(item) == &deleted && get_value(item) == &deleted);
}

static int resize(struct hash_table* ht, float factor)
{
    FILE* db = dbopen();
    if (!db) {
        LOG(LIB_LVL, CWARNING, "dbborrow failed, cannot access prime database");
        return 1;
    }
    size_t current_pi = pi(db, ht->capacity);
    long target_index = (long)(current_pi * factor);
    if (target_index < 1) target_index = 1;
    size_t new_capacity = nthprime(db, target_index);
    // FIX 1: Allow shrinking. 
    // Only skip if we are growing but calculated a smaller size (impossible with factor > 1)
    // or if we are shrinking but calculated a larger size (impossible with factor < 1)
    // Simplified: Just ensure we don't go below BASE_PRIME.
    if (new_capacity < BASE_PRIME)
        new_capacity = BASE_PRIME;
    if (new_capacity == ht->capacity) {
        LOG(LIB_LVL, CINFO, "Resize resulted in same capacity, skipping.");
        dbclose(db);
        return 0;
    }
    struct ht_item* old_items = ht->items;
    size_t old_capacity = ht->capacity;
    if (init_size_ht(ht, new_capacity) != 0) {
        LOG(LIB_LVL, CERROR, "init_size_ht failed");
        dbclose(db);
        return 1;
    }
    for (size_t i = 0; i < old_capacity; i++) {
        struct ht_item* curr_item = &old_items[i];
        if (!is_null(curr_item) && !is_deleted(curr_item)) {
            int attempts = 0;
            int index = ht->hc.hash(get_key(curr_item), ht->capacity, attempts);
            struct ht_item* curr_slot = &ht->items[index];
            while (!is_null(curr_slot)) {
                attempts++;
                index = ht->hc.hash(get_key(curr_item), ht->capacity, attempts);
                curr_slot = &ht->items[index];
            }
            *curr_slot = *curr_item;
            ht->size++; 
        }
    }
    free(old_items);
    dbclose(db);
    return 0;
}

static int resize_up(struct hash_table* ht, float load)
{
    if (load < UP_LOAD_RATIO)
        return 0;
    return resize(ht, FACTOR_UP);
}

static int resize_down(struct hash_table* ht, float load)
{
    if (load >= DOWN_LOAD_RATIO)
        return 0;
    return resize(ht, FACTOR_DOWN);
}

// *** Considerations *** //

/*

// Set either key or value, set_key and set_value wrap this, returns 0 if it succeeds, 1 otherwise
// Dont pass variables that would cause UB because they are passed into memcpy without sanitization
// Will introduce realloc for resetting objects later
// Leaves object's state the same as before the function call in case of failure, provided by init_item

static int set_field(void** _dest, size_t* _dest_size, const void* _src, size_t _src_size)
{
    void* new_field = malloc(_src_size);
    if (!new_field)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }

    memcpy(new_field, _src, _src_size);
    free(*_dest);
    *_dest = new_field;
    *_dest_size = _src_size;
    return 0;
}

*/