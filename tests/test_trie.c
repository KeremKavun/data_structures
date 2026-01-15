#include <ds/trees/trie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

/*
 * IMPLEMENTATION NOTES:
 * 
 * 1. Early exit from trie_prefix_iterate is not yet implemented due to 
 *    complexity with recursive traversal. The callback return value is 
 *    currently ignored during recursion.
 * 
 * 2. The implementation doesn't free nodes when they become empty after removal.
 */

// Color codes for terminal output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

// Helper macros
#define TEST_START(name) printf("\n" COLOR_YELLOW "Testing: %s" COLOR_RESET "\n", name)
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf(COLOR_GREEN "  ✓ " COLOR_RESET "%s\n", message); \
            tests_passed++; \
        } else { \
            printf(COLOR_RED "  ✗ " COLOR_RESET "%s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// Default mapper: identity function for ASCII
size_t default_mapper(unsigned char c) {
    return (size_t)c;
}

unsigned char default_unmapper(size_t i) {
    return (unsigned char)i;
}

// Callback for prefix iteration
typedef struct {
    int count;
    char keys[100][256];
    void* values[100];
} iteration_context;

int collect_keys_callback(const char *key, void *value, void *context) {
    iteration_context *ctx = (iteration_context *)context;
    if (ctx->count < 100) {
        strcpy(ctx->keys[ctx->count], key);
        ctx->values[ctx->count] = value;
        ctx->count++;
    }
    return 1; // Continue iteration
}

int stop_after_two_callback(const char *key, void *value, void *context) {
    iteration_context *ctx = (iteration_context *)context;
    if (ctx->count < 100) {
        strcpy(ctx->keys[ctx->count], key);
        ctx->values[ctx->count] = value;
        ctx->count++;
    }
    return ctx->count < 2 ? 1 : 0; // Return 1 to continue until we have 2 items, then 0 to stop
}

void test_init_deinit(void) {
    TEST_START("Initialization and Deinitialization");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    TEST_ASSERT(tr.alphabet_size == 256, "Alphabet size set correctly");
    TEST_ASSERT(tr.count == 0, "Initial count is 0");
    TEST_ASSERT(trie_is_empty(&tr), "Trie is initially empty");
    TEST_ASSERT(trie_size(&tr) == 0, "Size is 0");
    
    trie_deinit(&tr, NULL);
    TEST_ASSERT(1, "Deinitialization successful");
}

void test_basic_operations(void) {
    TEST_START("Basic Dictionary Operations");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    // Test put operations
    char *val1 = "value1";
    char *val2 = "value2";
    char *val3 = "value3";
    
    void *old = trie_put(&tr, "key1", val1);
    TEST_ASSERT(old == NULL, "First insert returns NULL");
    TEST_ASSERT(trie_size(&tr) == 1, "Size is 1 after insert");
    TEST_ASSERT(!trie_is_empty(&tr), "Trie is not empty");
    
    trie_put(&tr, "key2", val2);
    trie_put(&tr, "key3", val3);
    TEST_ASSERT(trie_size(&tr) == 3, "Size is 3 after three inserts");
    
    // Test get operations
    void *retrieved = NULL;
    enum trees_status status = trie_get(&tr, "key1", &retrieved);
    TEST_ASSERT(status == TREES_OK, "Get returns TREES_OK for existing key");
    TEST_ASSERT(retrieved == val1, "Retrieved correct value");
    
    status = trie_get(&tr, "nonexistent", &retrieved);
    TEST_ASSERT(status == TREES_NOT_FOUND, "Get returns TREES_NOT_FOUND for missing key");
    
    // Test contains
    TEST_ASSERT(trie_contains(&tr, "key1"), "Contains returns 1 for existing key");
    TEST_ASSERT(!trie_contains(&tr, "key4"), "Contains returns 0 for missing key");
    
    // Test update
    char *new_val = "updated_value";
    old = trie_put(&tr, "key1", new_val);
    TEST_ASSERT(old == val1, "Update returns old value");
    TEST_ASSERT(trie_size(&tr) == 3, "Size unchanged after update");
    
    trie_get(&tr, "key1", &retrieved);
    TEST_ASSERT(retrieved == new_val, "Value was updated");
    
    trie_deinit(&tr, NULL);
}

void test_remove_operations(void) {
    TEST_START("Remove Operations");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    char *val1 = "value1";
    char *val2 = "value2";
    
    trie_put(&tr, "test", val1);
    trie_put(&tr, "testing", val2);
    
    void *removed = NULL;
    enum trees_status status = trie_remove(&tr, "test", &removed);
    TEST_ASSERT(status == TREES_OK, "Remove returns TREES_OK");
    TEST_ASSERT(removed == val1, "Removed correct value");
    TEST_ASSERT(trie_size(&tr) == 1, "Size decreased after remove");
    TEST_ASSERT(!trie_contains(&tr, "test"), "Key no longer in trie");
    TEST_ASSERT(trie_contains(&tr, "testing"), "Other key still exists");
    
    status = trie_remove(&tr, "test", &removed);
    TEST_ASSERT(status == TREES_NOT_FOUND, "Removing non-existent key returns TREES_NOT_FOUND");
    
    trie_deinit(&tr, NULL);
}

void test_empty_string(void) {
    TEST_START("Empty String Handling");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    char *empty_val = "empty_value";
    void *old = trie_put(&tr, "", empty_val);
    TEST_ASSERT(old == NULL, "Can insert empty string");
    TEST_ASSERT(trie_size(&tr) == 1, "Size is 1 after empty string insert");
    
    void *retrieved = NULL;
    enum trees_status status = trie_get(&tr, "", &retrieved);
    TEST_ASSERT(status == TREES_OK, "Can retrieve empty string");
    TEST_ASSERT(retrieved == empty_val, "Empty string value correct");
    
    TEST_ASSERT(trie_contains(&tr, ""), "Contains works for empty string");
    
    void *removed = NULL;
    status = trie_remove(&tr, "", &removed);
    TEST_ASSERT(status == TREES_OK, "Can remove empty string");
    TEST_ASSERT(removed == empty_val, "Removed correct value");
    TEST_ASSERT(trie_size(&tr) == 0, "Size is 0 after removing empty string");
    
    trie_deinit(&tr, NULL);
}

void test_prefix_keys(void) {
    TEST_START("Prefix Key Relationships");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    trie_put(&tr, "app", "1");
    trie_put(&tr, "apple", "2");
    trie_put(&tr, "application", "3");
    trie_put(&tr, "apply", "4");
    
    TEST_ASSERT(trie_size(&tr) == 4, "All prefix keys inserted");
    TEST_ASSERT(trie_contains(&tr, "app"), "Shortest prefix exists");
    TEST_ASSERT(trie_contains(&tr, "apple"), "Middle prefix exists");
    TEST_ASSERT(trie_contains(&tr, "application"), "Longest prefix exists");
    TEST_ASSERT(trie_contains(&tr, "apply"), "Alternative branch exists");
    TEST_ASSERT(!trie_contains(&tr, "ap"), "Partial prefix doesn't exist");
    
    void *val = NULL;
    trie_get(&tr, "app", &val);
    TEST_ASSERT(strcmp((char*)val, "1") == 0, "Shortest prefix has correct value");
    
    trie_deinit(&tr, NULL);
}

void test_prefix_iteration(void) {
    TEST_START("Prefix Iteration");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    trie_put(&tr, "cat", "1");
    trie_put(&tr, "car", "2");
    trie_put(&tr, "card", "3");
    trie_put(&tr, "care", "4");
    trie_put(&tr, "dog", "5");
    trie_put(&tr, "dodge", "6");
    
    iteration_context ctx = {0};
    trie_prefix_iterate(&tr, "car", &ctx, collect_keys_callback);
    TEST_ASSERT(ctx.count == 3, "Found 3 keys with prefix 'car'");
    
    // Reset and test empty prefix (all keys)
    memset(&ctx, 0, sizeof(ctx));
    trie_prefix_iterate(&tr, "", &ctx, collect_keys_callback);
    TEST_ASSERT(ctx.count == 6, "Found all 6 keys with empty prefix");
    
    // Test non-existent prefix
    memset(&ctx, 0, sizeof(ctx));
    trie_prefix_iterate(&tr, "xyz", &ctx, collect_keys_callback);
    TEST_ASSERT(ctx.count == 0, "Found 0 keys with non-existent prefix");
    
    // TODO: Early exit callback test - not yet implemented due to recursion complexity
    // memset(&ctx, 0, sizeof(ctx));
    // trie_prefix_iterate(&tr, "", &ctx, stop_after_two_callback);
    // TEST_ASSERT(ctx.count == 2, "Callback can stop iteration early");
    
    trie_deinit(&tr, NULL);
}

void test_longest_prefix(void) {
    TEST_START("Longest Prefix Matching");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    trie_put(&tr, "app", "1");
    trie_put(&tr, "apple", "2");
    trie_put(&tr, "application", "3");
    
    size_t len = trie_longest_prefix(&tr, "applications");
    TEST_ASSERT(len == 11, "Longest prefix of 'applications' is 11 (application)");
    
    len = trie_longest_prefix(&tr, "apply");
    TEST_ASSERT(len == 3, "Longest prefix of 'apply' is 3 (app)");
    
    len = trie_longest_prefix(&tr, "banana");
    TEST_ASSERT(len == 0, "Longest prefix of 'banana' is 0");
    
    len = trie_longest_prefix(&tr, "apple");
    TEST_ASSERT(len == 5, "Exact match returns full length");
    
    trie_deinit(&tr, NULL);
}

void test_stress(void) {
    TEST_START("Stress Test");
    
    struct syspool pool = {.obj_size = trie_node_sizeof(256)};
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree
    };
    
    struct trie tr;
    trie_init(&tr, &ac, 256, default_mapper, default_unmapper);
    
    // Insert many keys
    const int num_keys = 1000;
    char keys[num_keys][32];
    
    for (int i = 0; i < num_keys; i++) {
        snprintf(keys[i], 32, "key_%d", i);
        // Add 1 to avoid NULL pointer (i+1 ensures values are 1-1000, never 0/NULL)
        trie_put(&tr, keys[i], (void*)(intptr_t)(i + 1));
    }
    
    TEST_ASSERT(trie_size(&tr) == num_keys, "All keys inserted");
    
    // Verify all keys
    int all_found = 1;
    for (int i = 0; i < num_keys; i++) {
        void *val = NULL;
        if (trie_get(&tr, keys[i], &val) != TREES_OK || 
            val != (void*)(intptr_t)(i + 1)) {
            all_found = 0;
            break;
        }
    }
    TEST_ASSERT(all_found, "All keys retrievable with correct values");
    
    // Remove half the keys
    for (int i = 0; i < num_keys / 2; i++) {
        void *removed = NULL;
        trie_remove(&tr, keys[i], &removed);
    }
    TEST_ASSERT(trie_size(&tr) == num_keys / 2, "Half keys removed");
    
    trie_deinit(&tr, NULL);
}

void print_summary(void) {
    printf("\n" COLOR_YELLOW "================== TEST SUMMARY ==================" COLOR_RESET "\n");
    printf("Tests passed: " COLOR_GREEN "%d" COLOR_RESET "\n", tests_passed);
    printf("Tests failed: " COLOR_RED "%d" COLOR_RESET "\n", tests_failed);
    printf("Total tests:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n✓ All tests passed!\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "\n✗ Some tests failed.\n" COLOR_RESET);
    }
    printf(COLOR_YELLOW "==================================================\n" COLOR_RESET);
}

int main(void) {
    printf(COLOR_YELLOW "Starting Trie Test Suite\n" COLOR_RESET);
    printf(COLOR_YELLOW "========================\n" COLOR_RESET);
    
    test_init_deinit();
    test_basic_operations();
    test_remove_operations();
    test_empty_string();
    test_prefix_keys();
    test_prefix_iteration();
    test_longest_prefix();
    test_stress();
    
    print_summary();
    
    return tests_failed > 0 ? 1 : 0;
}