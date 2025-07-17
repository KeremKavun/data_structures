#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hash_table.h"
#include <math.h>
#include "../prime/primes.h"

#define BASE_PRIMETH 16
#define HT_PRIME_1 151
#define HT_PRIME_2 217

static ht_item HT_DELETED_ITEM = {NULL, NULL};
static int initialized = 0;

static ht_item* ht_new_item(const char* key, const char* value)
{
    ht_item* item = malloc(sizeof(ht_item));

    item->key = strdup(key);
    item->value = strdup(value);

    if (item->key == NULL || item->value == NULL)
    {
        fprintf(stderr, "Memory allocation failed for key/value strings.\n");
        free(item->key);
        free(item->value);
        free(item);
        return NULL;
    }

    return item;
}

static ht_hash_table* ht_new_sized(const int base_primeth)
{
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    if (ht == NULL)
    {
        fprintf(stderr, "Memory allocation failed for hash table.\n");
        exit(EXIT_FAILURE);
    }
    ht->base_primeth = base_primeth;

    ht->size = pi(ht->base_primeth);

    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
    if (ht->items == NULL)
    {
        fprintf(stderr, "Memory allocation failed for hash table items.\n");
        free(ht);
        exit(EXIT_FAILURE);
    }
    return ht;
}

ht_hash_table* ht_new()
{
    // ******************************************************************************************** //
    if (!initialized)
        init();
    initialized++;
    // ******************************************************************************************** //
    return ht_new_sized(BASE_PRIMETH);
}

static void ht_del_item(ht_item* item)
{
    if (item == NULL && item != &HT_DELETED_ITEM) return; // added HT_DELETED_ITEM check
    free(item->key);
    free(item->value);
    free(item);
}

void ht_del_hash_table(ht_hash_table* ht)
{
    for (int i = 0; i < ht->size; i++)
    {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) // added HT_DELETED_ITEM check
            ht_del_item(item);
    }
    free(ht->items);
    free(ht);
}

static int ht_hash(const char* s, const int p, const int size)
{
    unsigned long hash = 0;
    while (*s)
        hash = hash * p + *s++;
    return (int) hash % size;
}

static int ht_get_hash(const char* s, const int num_buckets, const int attempts)
{
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + attempts * (hash_b + 1)) % num_buckets;
}

static void ht_resize_up(ht_hash_table* ht);
static void ht_resize_down(ht_hash_table* ht);

void ht_insert(ht_hash_table* ht, const char* key, const char* value)
{
    if (ht == NULL || key == NULL || value == NULL) return;

    const int load = ht->count * 100 / ht->size;
    if (load > 70)
        ht_resize_up(ht);

    int attempts = 0;
    int index = ht_get_hash(key, ht->size, attempts);
    ht_item* cur_item = ht->items[index];
    while (cur_item != NULL && cur_item != &HT_DELETED_ITEM)
    {
        if (strcmp(cur_item->key, key) == 0)
        {
            char* new_value = strdup(value);
            if (new_value == NULL)
            {
                fprintf(stderr, "Memory allocation failed for value update.\n");
                return;
            }
            free(cur_item->value);
            cur_item->value = new_value;
            return;
        }
        index = ht_get_hash(key, ht->size, ++attempts);
        cur_item = ht->items[index];
    }

    ht_item* new_item = ht_new_item(key, value);
    if (new_item == NULL)
    {
        fprintf(stderr, "Memory allocation failed for new item.\n");
        return;
    }

    ht->items[index] = new_item;
    ht->count++;
}

char* ht_search(ht_hash_table* ht, const char* key)
{
    if (ht == NULL || key == NULL) return NULL;

    int i = 0;
    int index = ht_get_hash(key, ht->size, i);
    ht_item* cur_item = ht->items[index];
    while (cur_item != NULL)
    {
        if (cur_item != &HT_DELETED_ITEM && strcmp(cur_item->key, key) == 0)
            return cur_item->value;
        index = ht_get_hash(key, ht->size, ++i);
        cur_item = ht->items[index];
    }
    return NULL; // Key not found
}

void ht_delete(ht_hash_table* ht, const char* key)
{
    if (ht == NULL || key == NULL) return;

    int i = 0;
    int index = ht_get_hash(key, ht->size, i);
    ht_item* cur_item = ht->items[index];
    while (cur_item != NULL)
    {
        if (cur_item != &HT_DELETED_ITEM && strcmp(cur_item->key, key) == 0)
        {
            ht_del_item(cur_item);
            ht->items[index] = &HT_DELETED_ITEM; // Mark as deleted
            ht->count--;

            const int load = ht->count * 100 / ht->size;
            if (load < 10)
                ht_resize_down(ht);

            // ******************************************************************************************** //
            if (initialized > 0)
            {
                initialized--;
                if (initialized == 0)
                    close();
            }
            // ******************************************************************************************** //

            return;
        }
        index = ht_get_hash(key, ht->size, ++i);
        cur_item = ht->items[index];
    }
}

static void ht_resize(ht_hash_table* ht, const int primeth)
{
    if (primeth <= BASE_PRIMETH)
        return;

    ht_hash_table* new_ht = ht_new_sized(primeth);
    for (int i = 0; i < ht->size; i++)
    {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM)
            ht_insert(new_ht, item->key, item->value);
    }

    ht->base_primeth = new_ht->base_primeth;
    ht->count = new_ht->count;

    const int old_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = old_size;

    ht_item** old_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = old_items;

    ht_del_hash_table(new_ht);
}

static void ht_resize_up(ht_hash_table* ht)
{
    ht_resize(ht, ht->base_primeth * 2);
}


static void ht_resize_down(ht_hash_table* ht)
{
    ht_resize(ht, ht->base_primeth / 2);
}