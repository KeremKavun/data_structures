#include <ds/hashs/hash_table.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

/*───────────────────────────────────────────────
 * Test Statistics & Utilities
 *───────────────────────────────────────────────*/
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) do { \
    if (condition) { \
        printf("  ✓ %s\n", message); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED: %s\n", message); \
        tests_failed++; \
    } \
} while(0)

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

/*───────────────────────────────────────────────
 * Hash & Compare Functions
 *───────────────────────────────────────────────*/
#define HT_PRIME_1 151
#define HT_PRIME_2 217

static size_t hash_str(const char* s, int p, int capacity)
{
    if (!s || capacity == 0) return 0;
    
    unsigned long long hash = 0;
    while (*s) {
        hash = hash * p + *s;
        s++;
    }
    return (size_t)(hash % capacity);
}

static size_t double_hash(const void* obj, size_t capacity, size_t attempts)
{
    if (!obj) return 0;
    
    size_t hash_a = hash_str((const char*)obj, HT_PRIME_1, capacity);
    size_t hash_b = hash_str((const char*)obj, HT_PRIME_2, capacity);
    return (hash_a + attempts * (hash_b + 1)) % capacity;
}

static int string_cmp(const void* obj1, const void* obj2)
{
    if (!obj1 || !obj2) return (obj1 == obj2) ? 0 : -1;
    return strcmp((const char*)obj1, (const char*)obj2);
}

/*───────────────────────────────────────────────
 * Integer Key Support (for testing different types)
 *───────────────────────────────────────────────*/
static size_t int_hash(const void* obj, size_t capacity, size_t attempts)
{
    if (!obj || capacity == 0) return 0;
    
    int value = *(const int*)obj;
    size_t hash_a = (size_t)(value * HT_PRIME_1) % capacity;
    size_t hash_b = (size_t)(value * HT_PRIME_2) % capacity;
    return (hash_a + attempts * (hash_b + 1)) % capacity;
}

static int int_cmp(const void* obj1, const void* obj2)
{
    if (!obj1 || !obj2) return (obj1 == obj2) ? 0 : -1;
    return *(const int*)obj1 - *(const int*)obj2;
}

/*───────────────────────────────────────────────
 * Deallocator & Iterator Functions
 *───────────────────────────────────────────────*/
static int walk_count = 0;

static void deallocator(void* key, void* value, void* userdata)
{
    (void)userdata;
    if (key) free(key);
    if (value) free(value);
}

static void walker(void* key, void* value, void* userdata)
{
    (void)userdata;
    if (key && value) {
        walk_count++;
        printf("  [%d] %s -> %d\n", walk_count, (char*)key, *(int*)value);
    }
}

static void sum_values(void* key, void* value, void* userdata)
{
    (void)key;
    if (value && userdata) {
        int* sum = (int*)userdata;
        *sum += *(int*)value;
    }
}

/*───────────────────────────────────────────────
 * Helper Functions
 *───────────────────────────────────────────────*/
static char* create_key(const char* prefix, int id)
{
    char* key = malloc(64);
    if (key) {
        snprintf(key, 64, "%s%d", prefix, id);
    }
    return key;
}

static int* create_value(int val)
{
    int* value = malloc(sizeof(int));
    if (value) {
        *value = val;
    }
    return value;
}

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

/* Test 1: Basic Creation and Destruction */
static void test_create_destroy(void)
{
    TEST_SECTION("Test 1: Basic Creation and Destruction");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    TEST_ASSERT(ht != NULL, "Hash table created successfully");
    TEST_ASSERT(hash_table_size(ht) == 0, "Initial size is 0");
    TEST_ASSERT(hash_table_capacity(ht) > 0, "Initial capacity is positive");
    
    hash_table_destroy(ht, NULL);
}

/* Test 2: Basic Insert and Search */
static void test_insert_search(void)
{
    TEST_SECTION("Test 2: Basic Insert and Search");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    char* key = create_key("test", 1);
    int* value = create_value(100);
    
    int result = hash_table_insert(ht, key, value);
    TEST_ASSERT(result == 0, "Insert returns 0 on success");
    TEST_ASSERT(hash_table_size(ht) == 1, "Size is 1 after insert");
    
    int* found = hash_table_search(ht, "test1");
    TEST_ASSERT(found != NULL, "Search finds inserted key");
    TEST_ASSERT(*found == 100, "Search returns correct value");
    
    int* not_found = hash_table_search(ht, "nonexistent");
    TEST_ASSERT(not_found == NULL, "Search returns NULL for missing key");
    
    hash_table_destroy(ht, NULL);
}

/* Test 3: Multiple Insertions */
static void test_multiple_inserts(void)
{
    TEST_SECTION("Test 3: Multiple Insertions");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    const int COUNT = 20;
    for (int i = 0; i < COUNT; i++) {
        char* key = create_key("item", i);
        int* value = create_value(i * 10);
        hash_table_insert(ht, key, value);
    }
    
    TEST_ASSERT(hash_table_size(ht) == COUNT, "Size matches insert count");
    
    // Verify all items
    bool all_found = true;
    for (int i = 0; i < COUNT; i++) {
        char keybuf[64];
        snprintf(keybuf, 64, "item%d", i);
        int* val = hash_table_search(ht, keybuf);
        if (!val || *val != i * 10) {
            all_found = false;
            break;
        }
    }
    TEST_ASSERT(all_found, "All inserted items found with correct values");
    
    hash_table_destroy(ht, NULL);
}

/* Test 4: Duplicate Key Handling */
static void test_duplicate_keys(void)
{
    TEST_SECTION("Test 4: Duplicate Key Handling");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    char* key1 = create_key("duplicate", 0);
    int* value1 = create_value(100);
    hash_table_insert(ht, key1, value1);
    
    char* key2 = create_key("duplicate", 0);
    int* value2 = create_value(200);
    hash_table_insert(ht, key2, value2);
    
    size_t size = hash_table_size(ht);
    int* found = hash_table_search(ht, "duplicate0");
    
    TEST_ASSERT(found != NULL, "Key exists after duplicate insert");
    // Note: Behavior depends on implementation (update vs ignore)
    printf("  Info: Size=%zu, Value=%d (implementation-dependent)\n", 
           size, found ? *found : -1);
    
    hash_table_destroy(ht, NULL);
}

/* Test 5: Remove Operations */
static void test_remove(void)
{
    TEST_SECTION("Test 5: Remove Operations");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    // Insert several items
    for (int i = 0; i < 5; i++) {
        char* key = create_key("key", i);
        int* value = create_value(i * 10);
        hash_table_insert(ht, key, value);
    }
    
    size_t initial_size = hash_table_size(ht);
    
    // Remove middle item
    int result = hash_table_remove(ht, "key2");
    TEST_ASSERT(result == 0, "Remove returns 0 on success");
    TEST_ASSERT(hash_table_size(ht) == initial_size - 1, "Size decreased by 1");
    TEST_ASSERT(hash_table_search(ht, "key2") == NULL, "Removed item not found");
    
    // Verify other items still exist
    TEST_ASSERT(hash_table_search(ht, "key0") != NULL, "Other items still exist");
    TEST_ASSERT(hash_table_search(ht, "key4") != NULL, "Other items still exist");
    
    // Remove non-existent key
    int result2 = hash_table_remove(ht, "nonexistent");
    TEST_ASSERT(result2 != 0, "Remove returns non-zero for missing key");
    
    hash_table_destroy(ht, NULL);
}

/* Test 6: Walk/Iteration */
static void test_walk(void)
{
    TEST_SECTION("Test 6: Walk/Iteration");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    const int COUNT = 5;
    int expected_sum = 0;
    
    for (int i = 0; i < COUNT; i++) {
        char* key = create_key("item", i);
        int* value = create_value(i);
        hash_table_insert(ht, key, value);
        expected_sum += i;
    }
    
    printf("  Walking through table:\n");
    walk_count = 0;
    hash_table_walk(ht, NULL, walker);
    TEST_ASSERT(walk_count == COUNT, "Walk visited all items");
    
    // Test context passing
    int actual_sum = 0;
    hash_table_walk(ht, &actual_sum, sum_values);
    TEST_ASSERT(actual_sum == expected_sum, "Context correctly passed to walker");
    
    hash_table_destroy(ht, NULL);
}

/* Test 7: Empty String Keys */
static void test_empty_strings(void)
{
    TEST_SECTION("Test 7: Empty and Special String Keys");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    // Empty string key
    char* empty_key = strdup("");
    int* value1 = create_value(999);
    int result = hash_table_insert(ht, empty_key, value1);
    TEST_ASSERT(result == 0, "Empty string key can be inserted");
    
    int* found = hash_table_search(ht, "");
    TEST_ASSERT(found != NULL && *found == 999, "Empty string key can be found");
    
    // Very long key
    char* long_key = malloc(1000);
    memset(long_key, 'a', 999);
    long_key[999] = '\0';
    int* value2 = create_value(888);
    hash_table_insert(ht, long_key, value2);
    
    int* found2 = hash_table_search(ht, long_key);
    TEST_ASSERT(found2 != NULL && *found2 == 888, "Very long key works");
    
    hash_table_destroy(ht, NULL);
}

/* Test 8: Stress Test - Large Dataset */
static void test_large_dataset(void)
{
    TEST_SECTION("Test 8: Stress Test - Large Dataset");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    const int LARGE_COUNT = 1000;
    
    // Insert many items
    for (int i = 0; i < LARGE_COUNT; i++) {
        char* key = create_key("large", i);
        int* value = create_value(i);
        hash_table_insert(ht, key, value);
    }
    
    TEST_ASSERT(hash_table_size(ht) == LARGE_COUNT, 
                "Large dataset: all items inserted");
    
    // Random access test
    bool random_access_ok = true;
    for (int i = 0; i < 100; i++) {
        int idx = rand() % LARGE_COUNT;
        char keybuf[64];
        snprintf(keybuf, 64, "large%d", idx);
        int* val = hash_table_search(ht, keybuf);
        if (!val || *val != idx) {
            random_access_ok = false;
            break;
        }
    }
    TEST_ASSERT(random_access_ok, "Random access works correctly");
    
    // Delete half the items
    for (int i = 0; i < LARGE_COUNT; i += 2) {
        char keybuf[64];
        snprintf(keybuf, 64, "large%d", i);
        hash_table_remove(ht, keybuf);
    }
    
    TEST_ASSERT(hash_table_size(ht) == LARGE_COUNT / 2, 
                "Half of items removed correctly");
    
    printf("  Info: Final capacity=%zu\n", hash_table_capacity(ht));
    
    hash_table_destroy(ht, NULL);
}

/* Test 9: Collision Handling */
static void test_collisions(void)
{
    TEST_SECTION("Test 9: Hash Collision Handling");
    
    struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    // Insert items that may cause collisions
    const char* collision_keys[] = {
        "abc", "acb", "bac", "bca", "cab", "cba"
    };
    const int COUNT = sizeof(collision_keys) / sizeof(collision_keys[0]);
    
    for (int i = 0; i < COUNT; i++) {
        char* key = strdup(collision_keys[i]);
        int* value = create_value(i);
        hash_table_insert(ht, key, value);
    }
    
    TEST_ASSERT(hash_table_size(ht) == COUNT, 
                "All collision keys inserted");
    
    // Verify all are retrievable
    bool all_retrievable = true;
    for (int i = 0; i < COUNT; i++) {
        int* val = hash_table_search(ht, collision_keys[i]);
        if (!val || *val != i) {
            all_retrievable = false;
            break;
        }
    }
    TEST_ASSERT(all_retrievable, "All collision keys retrievable");
    
    hash_table_destroy(ht, NULL);
}

/* Test 10: Integer Keys (Different Type) */
static void test_integer_keys(void)
{
    TEST_SECTION("Test 10: Integer Keys (Alternative Key Type)");
    
    struct hash_concept hc = { .hash = int_hash, .cmp_key = int_cmp };
    struct hash_table* ht = hash_table_create(&hc);
    
    const int COUNT = 10;
    for (int i = 0; i < COUNT; i++) {
        int* key = create_value(i * 100);
        int* value = create_value(i * 1000);
        hash_table_insert(ht, key, value);
    }
    
    TEST_ASSERT(hash_table_size(ht) == COUNT, "Integer keys inserted");
    
    // Search using temporary keys
    bool all_found = true;
    for (int i = 0; i < COUNT; i++) {
        int search_key = i * 100;
        int* val = hash_table_search(ht, &search_key);
        if (!val || *val != i * 1000) {
            all_found = false;
            break;
        }
    }
    TEST_ASSERT(all_found, "All integer keys found");
    
    hash_table_destroy(ht, NULL);
}

/*───────────────────────────────────────────────
 * Main Test Runner
 *───────────────────────────────────────────────*/
int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║   HASH TABLE COMPREHENSIVE TEST SUITE     ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    
    // Run all test cases
    test_create_destroy();
    test_insert_search();
    test_multiple_inserts();
    test_duplicate_keys();
    test_remove();
    test_walk();
    test_empty_strings();
    test_large_dataset();
    test_collisions();
    test_integer_keys();
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║              TEST SUMMARY                  ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  Passed: %-4d                             ║\n", tests_passed);
    printf("║  Failed: %-4d                             ║\n", tests_failed);
    printf("║  Total:  %-4d                             ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? 1 : 0;
}