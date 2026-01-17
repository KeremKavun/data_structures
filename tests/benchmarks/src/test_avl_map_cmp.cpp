/**
 * @file avl_map_comparison_intrusive.cpp
 * @brief Comprehensive comparison tests between intrusive AVL tree and std::map
 * 
 * Compile with:
 * g++ -std=c++17 -O2 avl_map_comparison_intrusive.cpp -I/path/to/include -L/path/to/lib -lds -o avl_test
 */

#include "../include/benchmark.hpp"
#include <ds/trees/avl.h>
#include <map>
#include <random>
#include <algorithm>
#include <cassert>
#include <vector>
#include <iostream>

// ============================================================================
// Test Data Structures with Intrusive Nodes
// ============================================================================

struct TestData {
    struct avl_node node;  // Intrusive node (MUST BE FIRST for container_of)
    int key;
    int value;
    
    TestData(int k = 0, int v = 0) : key(k), value(v) {
        // Properly zero-initialize for C code
        node.btree.parent = NULL;
        node.btree.left = NULL;
        node.btree.right = NULL;
    }
    
    // Get TestData from avl_node pointer
    static TestData* from_node(struct avl_node *n) {
        if (!n) return NULL;
        return reinterpret_cast<TestData*>(n);
    }
    
    static const TestData* from_node(const struct avl_node *n) {
        if (!n) return NULL;
        return reinterpret_cast<const TestData*>(n);
    }
};

// Comparison function for AVL tree (compares avl_nodes)
int compare_test_data(const struct bintree *a, const struct bintree *b) {
    const TestData *da = TestData::from_node((const struct avl_node*)a);
    const TestData *db = TestData::from_node((const struct avl_node*)b);
    return da->key - db->key;
}

// Comparison function for search (compares void* key with bintree node)
// Note: bintree_cmp_cb signature is (const void *key, const struct bintree *node)
int compare_test_data_search(const void *key, const struct bintree *node) {
    const TestData *dk = static_cast<const TestData*>(key);
    const TestData *dn = TestData::from_node((const struct avl_node*)node);
    return dk->key - dn->key;
}

// Destructor for cleanup
void destroy_test_data(void *data) {
    TestData *td = static_cast<TestData*>(data);
    delete td;
}

// Comparison for std::map
struct TestDataCmp {
    bool operator()(const TestData* a, const TestData* b) const {
        return a->key < b->key;
    }
};

// ============================================================================
// Test Fixtures
// ============================================================================

class AVLMapTest {
public:
    AVLMapTest() {
        avl_init(&avl_tree, compare_test_data);
    }

    ~AVLMapTest() {
        // Manually cleanup remaining nodes
        cleanup_tree(avl_tree.root);
        avl_tree.root = NULL;
        avl_tree.size = 0;
    }
    
    void cleanup_tree(struct avl_node *node) {
        if (!node) return;
        
        cleanup_tree(AVL_NODE(node->btree.left));
        cleanup_tree(AVL_NODE(node->btree.right));
        
        TestData *td = TestData::from_node(node);
        delete td;
    }

    struct avl avl_tree;
};

// ============================================================================
// Correctness Tests
// ============================================================================

void test_basic_operations() {
    std::cout << "\n[TEST] Basic Operations Correctness" << std::endl;
    
    AVLMapTest fixture;
    std::map<TestData*, int, TestDataCmp> map;
    
    // Test insertion
    const int N = 100;
    std::vector<TestData*> test_data;
    
    for (int i = 0; i < N; i++) {
        TestData *td = new TestData(i, i * 10);
        test_data.push_back(td);
        
        int status = avl_add(&fixture.avl_tree, &td->node);
        map[td] = td->value;
        
        assert(status == 0);  // 0 = success
        assert(avl_size(&fixture.avl_tree) == (size_t)(i + 1));
    }
    
    std::cout << "  ✓ Insertion: " << N << " elements" << std::endl;
    
    // Test search
    for (int i = 0; i < N; i++) {
        TestData query(i, 0);
        struct avl_node *found = avl_search(&fixture.avl_tree, &query, compare_test_data_search);
        TestData *found_data = TestData::from_node(found);
        
        if (found_data == NULL) {
            std::cerr << "Failed to find key: " << i << std::endl;
            std::cerr << "Tree size: " << avl_size(&fixture.avl_tree) << std::endl;
        }
        
        assert(found_data != NULL);
        assert(found_data->key == i);
        assert(found_data->value == i * 10);
    }
    
    std::cout << "  ✓ Search: All " << N << " elements found" << std::endl;
    
    // Test removal
    for (int i = 0; i < N; i += 2) {
        TestData query(i, 0);
        struct avl_node *found = avl_search(&fixture.avl_tree, &query, compare_test_data_search);
        assert(found != nullptr);
        
        avl_remove(&fixture.avl_tree, found);
        
        TestData *removed_data = TestData::from_node(found);
        assert(removed_data->key == i);
        
        map.erase(removed_data);
        delete removed_data;
    }
    
    assert(avl_size(&fixture.avl_tree) == N / 2);
    std::cout << "  ✓ Removal: " << N/2 << " elements removed" << std::endl;
    
    // Cleanup will be handled by fixture destructor
    std::cout << "  ✓ Final cleanup: Tree cleared" << std::endl;
}

void test_duplicate_insertion() {
    std::cout << "\n[TEST] Duplicate Insertion Handling" << std::endl;
    
    AVLMapTest fixture;
    
    TestData *td1 = new TestData(42, 100);
    TestData *td2 = new TestData(42, 200);
    
    int status1 = avl_add(&fixture.avl_tree, &td1->node);
    assert(status1 == 0);  // success
    assert(avl_size(&fixture.avl_tree) == 1);
    
    int status2 = avl_add(&fixture.avl_tree, &td2->node);
    assert(status2 == 1);  // duplicate
    assert(avl_size(&fixture.avl_tree) == 1);
    
    // td2 was rejected, so we need to delete it manually
    delete td2;
    
    std::cout << "  ✓ Duplicate correctly rejected" << std::endl;
    
    // td1 will be cleaned up by fixture destructor
}

void test_empty_tree_operations() {
    std::cout << "\n[TEST] Empty Tree Operations" << std::endl;
    
    AVLMapTest fixture;
    
    assert(avl_empty(&fixture.avl_tree));
    assert(avl_size(&fixture.avl_tree) == 0);
    
    TestData query(42, 0);
    struct avl_node *result = avl_search(&fixture.avl_tree, &query, compare_test_data_search);
    assert(result == NULL);
    
    std::cout << "  ✓ Empty tree operations handled correctly" << std::endl;
}

// ============================================================================
// Performance Benchmarks
// ============================================================================

BenchmarkResult benchmark_sequential_insertion(size_t n) {
    BenchmarkTimer timer;
    BenchmarkResult result;
    result.name = "Sequential Insert (" + std::to_string(n) + ")";
    result.operations = n;
    
    // Prepare test data
    std::vector<TestData*> test_data;
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test AVL
    {
        AVLMapTest fixture;
        timer.start();
        for (auto td : test_data) {
            avl_add(&fixture.avl_tree, &td->node);
        }
        timer.stop();
        result.avl_time_ms = timer.elapsed_ms();
        // Fixture destructor will clean up
    }
    
    // Recreate test data for std::map test
    test_data.clear();
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test std::map
    {
        std::map<TestData*, int, TestDataCmp> map;
        timer.start();
        for (auto td : test_data) {
            map[td] = td->value;
        }
        timer.stop();
        result.map_time_ms = timer.elapsed_ms();
    }
    
    // Cleanup
    for (auto td : test_data) {
        delete td;
    }
    
    result.speedup = result.map_time_ms / result.avl_time_ms;
    return result;
}

BenchmarkResult benchmark_random_insertion(size_t n) {
    BenchmarkTimer timer;
    BenchmarkResult result;
    result.name = "Random Insert (" + std::to_string(n) + ")";
    result.operations = n;
    
    // Prepare shuffled test data
    std::vector<TestData*> test_data;
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(test_data.begin(), test_data.end(), g);
    
    // Test AVL
    {
        AVLMapTest fixture;
        timer.start();
        for (auto td : test_data) {
            avl_add(&fixture.avl_tree, &td->node);
        }
        timer.stop();
        result.avl_time_ms = timer.elapsed_ms();
    }
    
    // Recreate and reshuffle for std::map
    test_data.clear();
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    std::shuffle(test_data.begin(), test_data.end(), g);
    
    // Test std::map
    {
        std::map<TestData*, int, TestDataCmp> map;
        timer.start();
        for (auto td : test_data) {
            map[td] = td->value;
        }
        timer.stop();
        result.map_time_ms = timer.elapsed_ms();
    }
    
    // Cleanup
    for (auto td : test_data) {
        delete td;
    }
    
    result.speedup = result.map_time_ms / result.avl_time_ms;
    return result;
}

BenchmarkResult benchmark_search(size_t n) {
    BenchmarkTimer timer;
    BenchmarkResult result;
    result.name = "Search (" + std::to_string(n) + ")";
    result.operations = n;
    
    // Prepare test data
    std::vector<TestData*> test_data;
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Build AVL tree
    AVLMapTest fixture;
    for (auto td : test_data) {
        avl_add(&fixture.avl_tree, &td->node);
    }
    
    // Build std::map
    std::map<TestData*, int, TestDataCmp> map;
    for (auto td : test_data) {
        map[td] = td->value;
    }
    
    // Test AVL search
    timer.start();
    for (size_t i = 0; i < n; i++) {
        TestData query(i, 0);
        volatile struct avl_node *found = avl_search(&fixture.avl_tree, &query, compare_test_data_search);
        (void)found; // Prevent optimization
    }
    timer.stop();
    result.avl_time_ms = timer.elapsed_ms();
    
    // Test std::map search
    timer.start();
    for (auto td : test_data) {
        volatile auto it = map.find(td);
        (void)it; // Prevent optimization
    }
    timer.stop();
    result.map_time_ms = timer.elapsed_ms();
    
    // Cleanup handled by fixture destructor
    
    result.speedup = result.map_time_ms / result.avl_time_ms;
    return result;
}

BenchmarkResult benchmark_removal(size_t n) {
    BenchmarkTimer timer;
    BenchmarkResult result;
    result.name = "Removal (" + std::to_string(n) + ")";
    result.operations = n;
    
    // Prepare test data
    std::vector<TestData*> test_data;
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test AVL removal
    {
        AVLMapTest fixture;
        for (auto td : test_data) {
            avl_add(&fixture.avl_tree, &td->node);
        }
        
        timer.start();
        for (auto td : test_data) {
            avl_remove(&fixture.avl_tree, &td->node);
            delete td;
        }
        timer.stop();
        result.avl_time_ms = timer.elapsed_ms();
    }
    
    // Recreate test data for std::map
    test_data.clear();
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test std::map removal
    {
        std::map<TestData*, int, TestDataCmp> map;
        for (auto td : test_data) {
            map[td] = td->value;
        }
        
        timer.start();
        for (auto td : test_data) {
            map.erase(td);
        }
        timer.stop();
        result.map_time_ms = timer.elapsed_ms();
    }
    
    // Cleanup
    for (auto td : test_data) {
        delete td;
    }
    
    result.speedup = result.map_time_ms / result.avl_time_ms;
    return result;
}

BenchmarkResult benchmark_mixed_operations(size_t n) {
    BenchmarkTimer timer;
    BenchmarkResult result;
    result.name = "Mixed Ops (" + std::to_string(n) + ")";
    result.operations = n * 3; // insert + search + remove
    
    std::vector<TestData*> test_data;
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test AVL
    {
        AVLMapTest fixture;
        timer.start();
        
        // Insert
        for (auto td : test_data) {
            avl_add(&fixture.avl_tree, &td->node);
        }
        
        // Search
        for (auto td : test_data) {
            TestData query(td->key, 0);
            avl_search(&fixture.avl_tree, &query, compare_test_data_search);
        }
        
        // Remove
        for (auto td : test_data) {
            avl_remove(&fixture.avl_tree, &td->node);
            delete td;
        }
        
        timer.stop();
        result.avl_time_ms = timer.elapsed_ms();
    }
    
    // Recreate test data
    test_data.clear();
    for (size_t i = 0; i < n; i++) {
        test_data.push_back(new TestData(i, i));
    }
    
    // Test std::map
    {
        std::map<TestData*, int, TestDataCmp> map;
        timer.start();
        
        // Insert
        for (auto td : test_data) {
            map[td] = td->value;
        }
        
        // Search
        for (auto td : test_data) {
            map.find(td);
        }
        
        // Remove
        for (auto td : test_data) {
            map.erase(td);
        }
        
        timer.stop();
        result.map_time_ms = timer.elapsed_ms();
    }
    
    // Cleanup
    for (auto td : test_data) {
        delete td;
    }
    
    result.speedup = result.map_time_ms / result.avl_time_ms;
    return result;
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║    AVL Tree (Intrusive) vs std::map Performance Comparison            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════════╝\n";
    
    // Run correctness tests
    test_basic_operations();
    test_duplicate_insertion();
    test_empty_tree_operations();
    
    // Run performance benchmarks
    BenchmarkSuite suite;
    
    std::cout << "\n[BENCHMARK] Running performance tests...\n" << std::endl;
    
    // Small datasets
    suite.add_result(benchmark_sequential_insertion(1000));
    suite.add_result(benchmark_random_insertion(1000));
    suite.add_result(benchmark_search(1000));
    suite.add_result(benchmark_removal(1000));
    suite.add_result(benchmark_mixed_operations(1000));
    
    // Medium datasets
    suite.add_result(benchmark_sequential_insertion(10000));
    suite.add_result(benchmark_random_insertion(10000));
    suite.add_result(benchmark_search(10000));
    suite.add_result(benchmark_removal(10000));
    suite.add_result(benchmark_mixed_operations(10000));
    
    // Large datasets
    suite.add_result(benchmark_sequential_insertion(100000));
    suite.add_result(benchmark_random_insertion(100000));
    suite.add_result(benchmark_search(100000));
    suite.add_result(benchmark_removal(100000));
    suite.add_result(benchmark_mixed_operations(100000));
    
    // Print summary
    suite.print_summary();
    
    std::cout << "\n✓ All tests completed successfully!\n" << std::endl;
    
    return 0;
}