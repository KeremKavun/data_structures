#include "../../prime/include/primes.h"
#include "../include/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define BASE_PRIME_INDEX 16
#define UP_LOAD_RATIO 0.7
#define DOWN_LOAD_RATIO 0.1

// ht_item helpers

// Init item, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure
static int init_item(struct ht_item* item, const void* _key, size_t _key_size, const void* _value, size_t _value_size);
// Set either key or value, set_key and set_value wrap this, returns 0 if it succeeds, 1 otherwise
// Dont pass variables that would cause UB because they are passed into memcpy without sanitization
// Will introduce realloc for resetting objects later
// Leaves object's state the same as before the function call in case of failure, provided by init_item
static int set_field(void** _dest, size_t* _dest_size, const void* _src, size_t _src_size);
// Set key, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by set_field
static int set_key(struct ht_item* item, const void* _key, size_t _key_size);
// Set value, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by set_field
static int set_value(struct ht_item* item, const void* _value, size_t _value_size);
// Free ht_item key value pair and set it to zero
static void free_item(struct ht_item* item);
// Returns 1 if the item is null (never assigned something), 0 otherwise
static int is_null(struct ht_item* item);
// Returns 1 if the item was deleted before, 0 otherwise
static int is_deleted(struct ht_item* item);

// hash_table helpers

// init_ht helper. Inits size and memory realted attributes.
// Leaves object's state the same as before the function call in case of failure
static int init_size_ht(struct hash_table* ht, size_t _prime_index);
// Resize hash_table, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize(struct hash_table* ht, size_t new_prime_index);
// Increase hash_table size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize_up(struct hash_table* ht);
// Decrease hash_table size, returns 0 if it succeeds, 1 otherwise
// Leaves object's state the same as before the function call in case of failure, provided by init_size_ht
static int resize_down(struct hash_table* ht);

static void* deleted;
static FILE* db;
static int initialized;

struct hash_table* init_ht
(
    struct hash_table* ht,
    size_t (*_hash) (const void* _key, size_t _capacity, size_t _attempts),
    int (*_cmp_key) (const void* _obj1, const void* _obj2)
)
{
    // *** static db call *** //
    if (!db)
    {
        db = dbopen();
        if (!db)
        {
            LOG(LIB_LVL, CWARNING, "dbopen failed, cannot access prime database");
            return NULL;
        }
        initialized++;
    }
    // ****** //
    if (init_size_ht(ht, BASE_PRIME_INDEX) != 0)
    {
        LOG(LIB_LVL, CERROR, "init_size_ht failed");
        return NULL;
    }
    ht->hash = _hash;
    ht->cmp_key = _cmp_key;
    return ht;
}

int insert_ht(struct hash_table* ht, const void* _key, size_t _key_size, const void* _value, size_t _value_size)
{
    if (ht == NULL || _key == NULL || _value == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return 1;
    }

    const float load = (float) ht->size / ht->capacity;
    if (load >= UP_LOAD_RATIO)
    {
        if (resize_up(ht) != 0)
        {
            LOG(LIB_LVL, CERROR, "Could not resize the hash_table up");
            return 1;
        }
    }

    int attempts = 0;
    int index = ht->hash(_key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    struct ht_item* first_deleted = NULL;
    while (!is_null(curr_item))
    {
        if (is_deleted(curr_item))
        {
            if (!first_deleted)
                first_deleted = curr_item;
        }
        else if (ht->cmp_key(curr_item->key, _key) == 0)
            return set_value(curr_item, _value, _value_size);
        attempts++;
        index = ht->hash(_key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }

    if (init_item((first_deleted) ? first_deleted : curr_item, _key, _key_size, _value, _value_size) != 0)
    {
        LOG(LIB_LVL, CERROR, "init_item failed");
        return 1;
    }
    ht->size++;
    return 0;
}

void* search_ht(struct hash_table* ht, const void* _key)
{
    if (ht == NULL || _key == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return NULL;
    }

    int attempts = 0;
    int index = ht->hash(_key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    while (!is_null(curr_item))
    {
        if (!is_deleted(curr_item) && ht->cmp_key(curr_item->key, _key) == 0)
            return curr_item->value;
        attempts++;
        index = ht->hash(_key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }
    return NULL; // Key not found
}

int delete_ht(struct hash_table* ht, const void* _key)
{
    if (ht == NULL || _key == NULL)
    {
        LOG(LIB_LVL, CERROR, "Invalid argument(s)");
        return 1;
    }

    int attempts = 0;
    int index = ht->hash(_key, ht->capacity, attempts);
    struct ht_item* curr_item = &ht->items[index];
    while (!is_null(curr_item))
    {
        if (!is_deleted(curr_item) && ht->cmp_key(curr_item->key, _key) == 0)
        {
            const float load = (float) ht->size / ht->capacity;
            if (load < DOWN_LOAD_RATIO)
            {
                if (resize_down(ht) != 0)
                {
                    LOG(LIB_LVL, CERROR, "Could not resize the hash_table down");
                    return 1;
                }
            }

            free_item(curr_item);
            ht->size--;

            // ******************************************************************************************** //
            initialized--;
            if (initialized == 0)
                dbclose(db);
            // ******************************************************************************************** //

            return 0;
        }
        attempts++;
        index = ht->hash(_key, ht->capacity, attempts);
        curr_item = &ht->items[index];
    }

    LOG(LIB_LVL, CERROR, "The key to be deleted couldnt be found");
    return 1;
}

void free_ht(struct hash_table* ht)
{
    for (size_t i = 0; i < ht->capacity; i++)
        if (!is_null(&ht->items[i]) && !is_deleted(&ht->items[i])) // added HT_DELETED_ITEM check
            free_item(&ht->items[i]);
    free(ht->items);
    // set everything to zero
    memset(ht, 0, sizeof(*ht));
}

// *** Helper functions *** //

static int init_size_ht(struct hash_table* ht, size_t _prime_index)
{
    prime_t _capacity = nthprime(db, _prime_index); // will make "noexcept"
    struct ht_item* _items = calloc(_capacity, sizeof(struct ht_item));
    if (!_items)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    ht->items = _items;
    ht->prime_index = _prime_index;
    ht->capacity = _capacity;
    ht->size = 0;
    return 0;
}

static int init_item(struct ht_item* item, const void* _key, size_t _key_size, const void* _value, size_t _value_size)
{
    struct ht_item old_item = *item;
    if (set_key(item, _key, _key_size) != 0)
    {
        LOG(LIB_LVL, CERROR, "set_key failed");
        return 1;
    }
    if (set_value(item, _value, _value_size) != 0)
    {
        LOG(LIB_LVL, CERROR, "set_value failed");
        free(item->key);
        *item = old_item;
        return 1;
    }

    return 0;
}

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

static int set_key(struct ht_item* item, const void* _key, size_t _key_size)
{
    return set_field(&item->key, &item->key_size, _key, _key_size);
}

static int set_value(struct ht_item* item, const void* _value, size_t _value_size)
{
    return set_field(&item->value, &item->value_size, _value, _value_size);
}

static void free_item(struct ht_item* item)
{
    free(item->key);
    free(item->value);
    item->key = &deleted;
    item->value = &deleted;
    item->key_size = 0;
    item->value_size = 0;
}

static int is_null(struct ht_item* item)
{
    static struct ht_item null_item;
    return !memcmp(&null_item, item, sizeof(struct ht_item));
}

static inline int is_deleted(struct ht_item* item)
{
    return (item->key == &deleted && item->value == &deleted);
}

static int resize(struct hash_table* ht, size_t new_prime_index)
{
    if (new_prime_index <= BASE_PRIME_INDEX)
    {
        LOG(LIB_LVL, CINFO, "Didnt resize down the hash table since it has minimum size defined by the implementation, returning success");
        return 0;
    }

    struct ht_item* old_items = ht->items;
    size_t old_capacity = ht->capacity;

    if (init_size_ht(ht, new_prime_index) != 0)
    {
        LOG(LIB_LVL, CERROR, "init_size_ht failed");
        return 1;
    }

    // Could not reuse insert_ht code because objects pointed to by key value pairs are already owned by this hash_table
    // So it is okay to copy, nothing redundant dynamic allocation and copy here
    for (size_t i = 0; i < old_capacity; i++)
    {
        struct ht_item* curr_item = &old_items[i];
        if (!is_null(curr_item) && !is_deleted(curr_item))
        {
            int attempts = 0;
            int index = ht->hash(curr_item->key, ht->capacity, attempts);
            struct ht_item* curr_slot = &ht->items[index];
            while (!is_null(curr_slot))
            {
                attempts++;
                index = ht->hash(curr_item->key, ht->capacity, attempts);
                curr_slot = &ht->items[index];
            }
            *curr_slot = *curr_item;
        }
    }

    free(old_items);
    return 0;
}

static int resize_up(struct hash_table* ht)
{
    return resize(ht, ht->prime_index * 2);
}

static int resize_down(struct hash_table* ht)
{
    return resize(ht, ht->prime_index / 2);
}