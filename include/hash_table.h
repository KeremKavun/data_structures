#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "../../debug/include/debug.h"
#include <stddef.h>

struct ht_item
{
    void* key;
    void* value;
    size_t key_size;
    size_t value_size;
};

struct hash_table
{
    struct ht_item* items;
    size_t prime_index;
    size_t capacity;
    size_t size;
    size_t (*hash) (const void* _key, size_t _capacity, size_t _attempts);
    int (*cmp_key) (const void* _obj1, const void* _obj2);
};

struct hash_table* init_ht
(
    struct hash_table* ht,
    size_t (*_hash) (const void* _key, size_t _capacity, size_t _attempts),
    int (*_cmp_key) (const void* _obj1, const void* _obj2)
);
// Inserts new item into hash table, returns 0 if it succedds, 1 otherwise
int insert_ht(struct hash_table* ht, const void* _key, size_t _key_size, const void* _value, size_t _value_size);
// Searches given key, returns void* value if it succedds, NULL otherwise
void* search_ht(struct hash_table* ht, const void* _key);
// Deletes given key, returns 0 if it succedds, 1 otherwise
int delete_ht(struct hash_table* ht, const void* _key);
// Frees contents of the hash_table
void free_ht(struct hash_table* ht);

#endif // HASH_TABLE_H

// sample hash implementation
/*
#define HT_PRIME_1 151
#define HT_PRIME_2 217

static int hash(const char* s, const int p, const int size)
{
    unsigned long long hash = 0;
    while (*s)
    {
        hash = hash * p + *s;
        s++;
    }
    return (int) hash % size;
}

// will be passed into init_ht as hash algorithm
static int double_hash(const char* s, const int num_buckets, const int attempts)
{
    const int hash_a = hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + attempts * (hash_b + 1)) % num_buckets;
}
*/