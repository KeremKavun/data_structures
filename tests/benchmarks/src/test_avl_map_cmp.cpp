/**
 * @file avl_map_comparison.cpp
 * @brief Comprehensive comparison tests between AVL tree and std::map
 * 
 * Compile with:
 * g++ -std=c++17 -O2 avl_map_comparison.cpp -I/path/to/include -L/path/to/lib -lds -o avl_test
 */

#include "../include/benchmark.hpp"
#include <ds/trees/avl.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include <map>
#include <random>
#include <algorithm>
#include <cassert>
#include <cstdlib>

// ============================================================================
// Test Data Structures
// ============================================================================

struct TestData {
    int key;
    int value;
    
    TestData(int k = 0, int v = 0) : key(k), value(v) {}
};

// Comparison function for AVL tree
int compare_test_data(const void *a, const void *b) {
    const TestData *da = static_cast<const TestData*>(a);
    const TestData *db = static_cast<const TestData*>(b);
    return da->key - db->key;
}

// Destructor for TestData (used with object_concept)
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
        // Initialize syspool for AVL node allocation
        pool.obj_size = avl_node_sizeof();
        
        // Setup allocator concept with syspool
        ac.allocator = &pool;
        ac.alloc = sysalloc;
        ac.free = sysfree;
        
        // Initialize AVL tree
        avl_init(&avl_tree, compare_test_data, &ac);
    }

    ~AVLMapTest() {
        // Setup object concept for cleanup
        struct object_concept oc;
        oc.init = nullptr;  // Not needed for deinit
        oc.deinit = destroy_test_data;
        
        // Deinitialize AVL tree (will call deinit on all remaining objects)
        avl_deinit(&avl_tree, &oc);
    }

    struct avl avl_tree;
    struct syspool pool;
    struct allocator_concept ac;
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
        
        auto status = avl_add(&fixture.avl_tree, td);
        map[td] = td->value;
        
        assert(status == TREES_OK);
        assert(avl_size(&fixture.avl_tree) == (size_t)(i + 1));
    }
    
    std::cout << "  ✓ Insertion: " << N << " elements" << std::endl;
    
    // Test search
    for (int i = 0; i < N; i++) {
        TestData query(i, 0);
        TestData *found = static_cast<TestData*>(avl_search(&fixture.avl_tree, &query));
        
        assert(found != nullptr);
        assert(found->key == i);
        assert(found->value == i * 10);
    }
    
    std::cout << "  ✓ Search: All " << N << " elements found" << std::endl;
    
    // Test removal
    for (int i = 0; i < N; i += 2) {
        TestData query(i, 0);
        TestData *removed = static_cast<TestData*>(avl_remove(&fixture.avl_tree, &query));
        
        assert(removed != nullptr);
        assert(removed->key == i);
        
        map.erase(removed);
        delete removed;
    }
    
    assert(avl_size(&fixture.avl_tree) == N / 2);
    std::cout << "  ✓ Removal: " << N/2 << " elements removed" << std::endl;
    
    // Cleanup remaining (will be handled by fixture destructor)
    for (auto& pair : map) {
        avl_remove(&fixture.avl_tree, pair.first);
        // Don't delete here - let avl_deinit handle it via object_concept
    }
    
    std::cout << "  ✓ Final cleanup: Tree cleared" << std::endl;
}

void test_duplicate_insertion() {
    std::cout << "\n[TEST] Duplicate Insertion Handling" << std::endl;
    
    AVLMapTest fixture;
    
    TestData *td1 = new TestData(42, 100);
    TestData *td2 = new TestData(42, 200);
    
    auto status1 = avl_add(&fixture.avl_tree, td1);
    assert(status1 == TREES_OK);
    assert(avl_size(&fixture.avl_tree) == 1);
    
    auto status2 = avl_add(&fixture.avl_tree, td2);
    assert(status2 == TREES_DUPLICATE_KEY);
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
    void *result = avl_search(&fixture.avl_tree, &query);
    assert(result == nullptr);
    
    void *removed = avl_remove(&fixture.avl_tree, &query);
    assert(removed == nullptr);
    
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
            avl_add(&fixture.avl_tree, td);
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
            avl_add(&fixture.avl_tree, td);
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
        avl_add(&fixture.avl_tree, td);
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
        volatile void *found = avl_search(&fixture.avl_tree, &query);
        (void)found; // Prevent optimization
    }
    timer.stop();
    result.avl_time_ms = timer.elapsed_ms();
    
    // Test std::map search (by value comparison, not pointer)
    timer.start();
    for (auto td : test_data) {
        volatile auto it = map.find(td);
        (void)it; // Prevent optimization
    }
    timer.stop();
    result.map_time_ms = timer.elapsed_ms();
    
    // Don't delete test_data here - let fixture destructor handle it
    
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
            avl_add(&fixture.avl_tree, td);
        }
        
        timer.start();
        for (auto td : test_data) {
            void *removed = avl_remove(&fixture.avl_tree, td);
            delete static_cast<TestData*>(removed);
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
            avl_add(&fixture.avl_tree, td);
        }
        
        // Search
        for (auto td : test_data) {
            avl_search(&fixture.avl_tree, td);
        }
        
        // Remove
        for (auto td : test_data) {
            void *removed = avl_remove(&fixture.avl_tree, td);
            delete static_cast<TestData*>(removed);
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
    std::cout << "║         AVL Tree vs std::map Performance Comparison                    ║\n";
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