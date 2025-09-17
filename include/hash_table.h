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

size_t hash_str(const char* s, int p, int capacity)
{
   unsigned long long hash = 0;
   while (*s)
   {
      hash = hash * p + *s;
      s++;
   }
   return (size_t) hash % capacity;
}

// will be passed into init_ht as hash algorithm
size_t double_hash(const void* obj, size_t capacity, size_t attempts)
{
   size_t hash_a = hash_str((const char*) obj, HT_PRIME_1, capacity);
   size_t hash_b = hash_str((const char*) obj, HT_PRIME_2, capacity);
   return (hash_a + attempts * (hash_b + 1)) % capacity;
}

// Simple string compare function
int string_cmp(const void* _obj1, const void* _obj2)
{
   return strcmp((const char*)_obj1, (const char*)_obj2);
}
*/