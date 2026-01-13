#include "../../../prime/include/primes.h"
#include <ds/hashs/hash_set.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BASE_PRIME 53
#define UP_LOAD_RATIO 0.7
#define DOWN_LOAD_RATIO 0.1
#define FACTOR_UP 2
#define FACTOR_DOWN 0.5

struct hash_set
{
   void** items;
   size_t capacity;
   size_t size;
   size_t (*hash) (const void* key, size_t capacity, size_t attempts);
   int (*cmp_key) (const void* obj1, const void* obj2);
};

// ht_item helpers

static void set_item(void** item, void* value);
static const void* get_item(void** item);
// Free ht_item key value pair and set it to zero
static void free_item(void** item);
// Returns 1 if the item is null (never assigned something), 0 oth  erwise
static int is_null(void** item);
// Returns 1 if the item was deleted before, 0 otherwise
static int is_deleted(void** item);

// hash_set helpers

static void hash_set_free_content(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata));
// hash_set helper. Inits size and memory realted attributes.
// Leaves object's state the same as before the function call in case of failure
static int hash_set_init_size(struct hash_set* hs, size_t capacity);
// Resize hash_set, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by hash_set_init_size
static int resize(struct hash_set* hs, float factor);
// Increase hash_set size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by hash_set_init_size
static int resize_up(struct hash_set* hs);
// Decrease hash_set size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by hash_set_init_size
static int resize_down(struct hash_set* hs);

static void* deleted;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int hash_set_init(struct hash_set* hs, size_t (*hash) (const void* key, size_t capacity, size_t attempts), int (*cmp_key) (const void* obj1, const void* obj2))
{
    if (hash_set_init_size(hs, BASE_PRIME) != 0)
    {
        LOG(LIB_LVL, CERROR, "hash_set_init_size failed");
        return 1;
    }
    hs->hash = hash;
    hs->cmp_key = cmp_key;
    return 0;
}

struct hash_set* hash_set_create(size_t (*hash) (const void* key, size_t capacity, size_t attempts), int (*cmp_key) (const void* obj1, const void* obj2))
{
    struct hash_set* hs = malloc(sizeof(*hs));
    if (hs == NULL)
    {
        LOG(LIB_LVL, CERROR, "malloc failed");
        return NULL;
    }
    if (hash_set_init(hs, hash, cmp_key) != 0)
    {
        LOG(LIB_LVL, CERROR, "hash_set_init failed");
        free(hs);
        return NULL;
    }
    return hs;
}

void hash_set_deinit(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    hash_set_free_content(hs, userdata, deallocator);
    // set everything to zero
    memset(hs, 0, sizeof(*hs));
}

void hash_set_destroy(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    hash_set_deinit(hs, userdata, deallocator);
    free(hs);
}

/*───────────────────────────────────────────────
 * Insertion & removal
 *───────────────────────────────────────────────*/

int hash_set_insert(struct hash_set* hs, void* item)
{
    if (hs == NULL || item == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return 1;
    }
    const float load = (float) hs->size / hs->capacity;
    if (load >= UP_LOAD_RATIO)
    {
        if (resize_up(hs) != 0)
        {
            LOG(LIB_LVL, CERROR, "Could not resize the hash_set up");
            return 1;
        }
    }
    int attempts = 0;
    int index = hs->hash(item, hs->capacity, attempts);
    void** curr_item = &hs->items[index];
    void** first_deleted = NULL;
    while (!is_null(curr_item))
    {
        if (is_deleted(curr_item))
        {
            if (!first_deleted)
                first_deleted = curr_item;
        }
        else if (hs->cmp_key(get_item(curr_item), item) == 0)
        {
            set_item(curr_item, item);
            return 0;
        }
        attempts++;
        index = hs->hash(item, hs->capacity, attempts);
        curr_item = &hs->items[index];
    }
    set_item((first_deleted) ? first_deleted : curr_item, item);
    hs->size++;
    return 0;
}

int hash_set_delete(struct hash_set* hs, const void* item)
{
    if (hs == NULL || item == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return 1;
    }
    int attempts = 0;
    int index = hs->hash(item, hs->capacity, attempts);
    void** curr_item = &hs->items[index];
    while (!is_null(curr_item))
    {
        if (!is_deleted(curr_item) && hs->cmp_key(get_item(curr_item), item) == 0)
        {
            // save current because possible resize will invalidate curr_item
            void* copy_curr = *curr_item;

            free_item(curr_item);
            hs->size--;

            const float load = (float) hs->size / hs->capacity;
            if (load < DOWN_LOAD_RATIO)
            {
                if (resize_down(hs) != 0)
                {
                    LOG(LIB_LVL, CERROR, "Could not resize the hash_set down");
                    *curr_item = copy_curr;
                    return 1;
                }
            }
            return 0;
        }
        attempts++;
        index = hs->hash(item, hs->capacity, attempts);
        curr_item = &hs->items[index];
    }
    LOG(LIB_LVL, CERROR, "The key to be deleted couldnt be found");
    return 1;
}

/*───────────────────────────────────────────────
 * Search
 *───────────────────────────────────────────────*/

void* hash_set_search(struct hash_set* hs, const void* item)
{
    if (hs == NULL || item == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return NULL;
    }
    int attempts = 0;
    int index = hs->hash(item, hs->capacity, attempts);
    void** curr_item = &hs->items[index];
    while (!is_null(curr_item))
    {
        if (!is_deleted(curr_item) && hs->cmp_key(get_item(curr_item), item) == 0)
            return *curr_item;
        attempts++;
        index = hs->hash(item, hs->capacity, attempts);
        curr_item = &hs->items[index];
    }
    return NULL; // Key not found
}

/*───────────────────────────────────────────────
 * Iteration
 *───────────────────────────────────────────────*/

void hash_set_walk(struct hash_set* hs, void* userdata, void (*exec) (void* item, void* userdata))
{
    for (size_t i = 0; i < hs->capacity; i++)
        if (!is_null(&hs->items[i]) && !is_deleted(&hs->items[i]))
            exec(hs->items[i], userdata);
}

// *** Helper functions *** //

static int hash_set_init_size(struct hash_set* hs, size_t capacity)
{
    FILE* db = dbborrow();
    if (!db)
    {
        LOG(LIB_LVL, CWARNING, "dbborrow failed, cannot access prime database");
        return 1;
    }
    void* items = calloc(capacity, sizeof(void*));
    if (!items)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    hs->items = items;
    hs->capacity = capacity;
    hs->size = 0;
    return 0;
}

static void set_item(void** item, void* value)
{
    *item = value;
}

static const void* get_item(void** item)
{
    return *item;
}

static void free_item(void** item)
{
    set_item(item, &deleted);
}

static int is_null(void** item)
{
    return get_item(item) == NULL;
}

static inline int is_deleted(void** item)
{
    return (get_item(item) == &deleted);
}

static void hash_set_free_content(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    if (deallocator)
        hash_set_walk(hs, userdata, deallocator);
    free(hs->items);
}

static int resize(struct hash_set* hs, float factor)
{
    FILE* db = dbborrow();
    if (!db)
    {
        LOG(LIB_LVL, CWARNING, "dbborrow failed, cannot access prime database");
        return 1;
    }
    size_t new_capacity = nthprime(db, (long) (pi(db, hs->capacity)) * factor);
    if (new_capacity <= BASE_PRIME)
    {
        LOG(LIB_LVL, CINFO, "Didnt resize down the hash table since it has minimum size defined by the implementation, returning success");
        return 0;
    }
    void** old_items = hs->items;
    size_t old_capacity = hs->capacity;
    if (hash_set_init_size(hs, nthprime(db, (long) (pi(db, old_capacity)) * factor)) != 0)
    {
        LOG(LIB_LVL, CERROR, "hash_set_init_size failed");
        return 1;
    }
    // Could not reuse hash_table_insert code because objects pointed to by key value pairs are already owned by this hash_set
    // So it is okay to copy, nothing redundant dynamic allocation and copy here
    for (size_t i = 0; i < old_capacity; i++)
    {
        void** curr_item = &old_items[i];
        if (!is_null(curr_item) && !is_deleted(curr_item))
        {
            int attempts = 0;
            int index = hs->hash(*curr_item, hs->capacity, attempts);
            void** curr_slot = &hs->items[index];
            while (!is_null(curr_slot))
            {
                attempts++;
                index = hs->hash(*curr_item, hs->capacity, attempts);
                curr_slot = &hs->items[index];
            }
            *curr_slot = *curr_item;
        }
    }
    free(old_items);
    return 0;
}

static int resize_up(struct hash_set* hs)
{
    return resize(hs, FACTOR_UP);
}

static int resize_down(struct hash_set* hs)
{
    return resize(hs, FACTOR_DOWN);
}