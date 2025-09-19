#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

struct ht_item;

struct hash_table
{
   struct ht_item* items;
   size_t capacity;
   size_t size;
   size_t (*hash) (const void* key, size_t capacity, size_t attempts);
   int (*cmp_key) (const void* obj1, const void* obj2);
};

struct hash_table* init_ht
(
   struct hash_table* ht,
   size_t (*hash) (const void* key, size_t capacity, size_t attempts),
   int (*cmp_key) (const void* obj1, const void* obj2)
);
// Inserts new item into hash table, returns 0 if it succedds, 1 otherwise
int insert_ht(struct hash_table* ht, void* key, void* value);
// Searches given key, returns void* value if it succedds, NULL otherwise
void* search_ht(struct hash_table* ht, const void* key);
// Deletes given key, returns 0 if it succedds, 1 otherwise
int delete_ht(struct hash_table* ht, const void* key);
// Walks through hash_table and executes given function ptr on objects' pointers stored in the hash_table
void walk_ht(struct hash_table* ht, void* userdata, void (*exec) (void* key, void* value, void* userdata));
// Frees contents of the hash_table, but it shouldnt be used if a key or value is allocated in the heap and
// pointer to that space is stored only here, otherwise you will lose the pointer and never able to free the resource
void free_ht(struct hash_table* ht, void* userdata, void (*deallocator) (void* key, void* value, void* userdata));

#ifdef __cplusplus
}
#endif

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