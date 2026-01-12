#include "../include/hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*───────────────────────────────────────────────
 * Hash & Compare
 *───────────────────────────────────────────────*/

#define HT_PRIME_1 151
#define HT_PRIME_2 217

static size_t hash_str(const char* s, int p, int capacity)
{
    unsigned long long hash = 0;
    while (*s)
    {
        hash = hash * p + *s;
        s++;
    }
    return (size_t)(hash % capacity);
}

static size_t double_hash(const void* obj, size_t capacity, size_t attempts)
{
    size_t hash_a = hash_str((const char*)obj, HT_PRIME_1, capacity);
    size_t hash_b = hash_str((const char*)obj, HT_PRIME_2, capacity);
    return (hash_a + attempts * (hash_b + 1)) % capacity;
}

static int string_cmp(const void* _obj1, const void* _obj2)
{
    return strcmp((const char*)_obj1, (const char*)_obj2);
}

/*───────────────────────────────────────────────
 * Deallocator & Iterator
 *───────────────────────────────────────────────*/

static void deallocator(void* key, void* value, void* userdata)
{
    (void)userdata;
    printf("Freeing key=\"%s\", value=%d\n", (char*)key, *(int*)value);
    free(key);
    free(value);
}

static void walker(void* key, void* value, void* userdata)
{
    (void)userdata;
    printf("walk: %s -> %d\n", (char*)key, *(int*)value);
}

/*───────────────────────────────────────────────
 * Test
 *───────────────────────────────────────────────*/

int main(void)
{
    printf("=== hash_table test start ===\n");

    hash_table_t* ht = hash_table_create(double_hash, string_cmp);
    assert(ht);

    // Insert key-value pairs
    for (int i = 0; i < 5; i++) {
        char* key = malloc(16);
        sprintf(key, "key%d", i);

        int* value = malloc(sizeof(int));
        *value = i * 10;

        assert(hash_table_insert(ht, key, value) == 0);
    }

    printf("Inserted 5 elements. Size=%zu Capacity=%zu\n",
           hash_table_size(ht),
           hash_table_capacity(ht));

    // Search for a few keys
    for (int i = 0; i < 5; i++) {
        char keybuf[16];
        sprintf(keybuf, "key%d", i);
        int* val = hash_table_search(ht, keybuf);
        assert(val);
        printf("Found: %s -> %d\n", keybuf, *val);
    }

    // Search for non-existent key
    assert(hash_table_search(ht, "keyX") == NULL);

    // Walk through table
    printf("\nWalking table:\n");
    hash_table_walk(ht, NULL, walker);

    // Delete one key
    assert(hash_table_delete(ht, "key2") == 0);
    assert(hash_table_search(ht, "key2") == NULL);
    printf("\nDeleted key2. New size=%zu\n", hash_table_size(ht));

    // Clean up
    printf("\nDeinitializing hash table...\n");
    hash_table_destroy(ht, NULL, deallocator);

    printf("=== hash_table test finished ===\n");
    return 0;
}