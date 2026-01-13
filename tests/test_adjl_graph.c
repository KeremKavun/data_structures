#include <ds/graphs/adjl_graph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("✓ PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("✗ FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// Sample data structures for testing
typedef struct {
    int id;
    char name[32];
} Person;

// Comparison function for Person
int person_compare(const void *key, const void *data) {
    const Person *k = (const Person *)key;
    const Person *d = (const Person *)data;
    return k->id - d->id;
}

// Object concept implementation for cleanup
void person_deinit(void *obj) {
    free(obj);
}

struct object_concept person_oc = {
    .deinit = person_deinit
};

// Helper function to create a person
Person* create_person(int id, const char *name) {
    Person *p = malloc(sizeof(Person));
    if (p) {
        p->id = id;
        strncpy(p->name, name, sizeof(p->name) - 1);
        p->name[sizeof(p->name) - 1] = '\0';
    }
    return p;
}

// Traversal handler for testing
typedef struct {
    int count;
    int ids[100];
} TraversalContext;

void count_handler(void *item, void *context) {
    Person *p = (Person *)item;
    TraversalContext *ctx = (TraversalContext *)context;
    if (ctx->count < 100) {
        ctx->ids[ctx->count++] = p->id;
    }
}

// Test functions
void test_graph_creation() {
    printf("\n=== Testing Graph Creation ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    TEST_ASSERT(graph != NULL, "Graph creation");
    TEST_ASSERT(adjl_graph_empty(graph) == 1, "Empty graph check");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 0, "Initial vertex count is 0");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 0, "Initial edge count is 0");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_operations() {
    printf("\n=== Testing Vertex Operations ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    
    // Add vertices
    int result = adjl_graph_add_vertex(graph, p1);
    TEST_ASSERT(result == 0, "Add first vertex");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 1, "Vertex count after first add");
    
    result = adjl_graph_add_vertex(graph, p2);
    TEST_ASSERT(result == 0, "Add second vertex");
    
    result = adjl_graph_add_vertex(graph, p3);
    TEST_ASSERT(result == 0, "Add third vertex");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 3, "Vertex count after three adds");
    TEST_ASSERT(adjl_graph_empty(graph) == 0, "Graph not empty after adds");
    
    // Search for vertices
    Person search_key = {2, ""};
    struct adjl_vertex *found = adjl_graph_search(graph, &search_key);
    TEST_ASSERT(found != NULL, "Search for existing vertex");
    
    Person *found_data = (Person *)adjl_vertex_get_data(found);
    TEST_ASSERT(found_data != NULL && found_data->id == 2, "Retrieved correct vertex data");
    
    search_key.id = 99;
    found = adjl_graph_search(graph, &search_key);
    TEST_ASSERT(found == NULL, "Search for non-existing vertex returns NULL");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_arc_operations() {
    printf("\n=== Testing Arc Operations ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    
    // Add arcs: 1->2, 1->3, 2->3
    int result = adjl_graph_add_arc(graph, v1, v2);
    TEST_ASSERT(result == 0, "Add arc 1->2");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 1, "Edge count after first arc");
    
    result = adjl_graph_add_arc(graph, v1, v3);
    TEST_ASSERT(result == 0, "Add arc 1->3");
    
    result = adjl_graph_add_arc(graph, v2, v3);
    TEST_ASSERT(result == 0, "Add arc 2->3");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 3, "Edge count after three arcs");
    
    // Check degrees
    TEST_ASSERT(adjl_vertex_outdegree(v1) == 2, "Vertex 1 outdegree is 2");
    TEST_ASSERT(adjl_vertex_indegree(v1) == 0, "Vertex 1 indegree is 0");
    TEST_ASSERT(adjl_vertex_outdegree(v2) == 1, "Vertex 2 outdegree is 1");
    TEST_ASSERT(adjl_vertex_indegree(v2) == 1, "Vertex 2 indegree is 1");
    TEST_ASSERT(adjl_vertex_outdegree(v3) == 0, "Vertex 3 outdegree is 0");
    TEST_ASSERT(adjl_vertex_indegree(v3) == 2, "Vertex 3 indegree is 2");
    
    // Remove an arc
    result = adjl_graph_remove_arc(graph, v1, v2);
    TEST_ASSERT(result == 0, "Remove arc 1->2");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 2, "Edge count after removal");
    TEST_ASSERT(adjl_vertex_outdegree(v1) == 1, "Vertex 1 outdegree after arc removal");
    TEST_ASSERT(adjl_vertex_indegree(v2) == 0, "Vertex 2 indegree after arc removal");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_iterator() {
    printf("\n=== Testing Vertex Iterator ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    
    adjl_vertex_iterator it;
    adjl_v_iter_init(graph, &it);
    
    int count = 0;
    int id_sum = 0;
    void *data;
    
    while ((data = adjl_v_iter_next(&it)) != NULL) {
        Person *p = (Person *)data;
        count++;
        id_sum += p->id;
    }
    
    TEST_ASSERT(count == 3, "Iterator visited all 3 vertices");
    TEST_ASSERT(id_sum == 6, "Iterator visited correct vertices (sum of ids)");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_outgoing_neighbor_iterator() {
    printf("\n=== Testing Outgoing Neighbor Iterator ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    Person *p4 = create_person(4, "Dave");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    
    // Create arcs: 1->2, 1->3, 1->4
    adjl_graph_add_arc(graph, v1, v2);
    adjl_graph_add_arc(graph, v1, v3);
    adjl_graph_add_arc(graph, v1, v4);
    
    adjl_out_iter oit;
    adjl_out_iter_init(v1, &oit);
    
    int count = 0;
    int id_sum = 0;
    int ids_found[3] = {0};
    void *data;
    
    while ((data = adjl_out_iter_next(&oit)) != NULL) {
        Person *p = (Person *)data;
        count++;
        id_sum += p->id;
        if (p->id >= 2 && p->id <= 4) {
            ids_found[p->id - 2] = 1;
        }
    }
    
    TEST_ASSERT(count == 3, "Outgoing iterator visited all 3 neighbors");
    TEST_ASSERT(id_sum == 9, "Outgoing iterator visited correct neighbors (2+3+4)");
    TEST_ASSERT(ids_found[0] && ids_found[1] && ids_found[2], 
                "Outgoing iterator found all expected IDs");
    
    // Test vertex with no outgoing neighbors
    adjl_out_iter_init(v4, &oit);
    count = 0;
    while ((data = adjl_out_iter_next(&oit)) != NULL) {
        count++;
    }
    TEST_ASSERT(count == 0, "Vertex with no outgoing neighbors returns empty iteration");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_incoming_neighbor_iterator() {
    printf("\n=== Testing Incoming Neighbor Iterator ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    Person *p4 = create_person(4, "Dave");
    Person *p5 = create_person(5, "Eve");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    adjl_graph_add_vertex(graph, p5);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""}, key5 = {5, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    struct adjl_vertex *v5 = adjl_graph_search(graph, &key5);
    
    // Create arcs: 1->4, 2->4, 3->4 (multiple vertices pointing to v4)
    adjl_graph_add_arc(graph, v1, v4);
    adjl_graph_add_arc(graph, v2, v4);
    adjl_graph_add_arc(graph, v3, v4);
    
    adjl_in_iter iit;
    adjl_in_iter_init(graph, v4, &iit);
    
    int count = 0;
    int id_sum = 0;
    int ids_found[3] = {0};
    void *data;
    
    while ((data = adjl_in_iter_next(&iit)) != NULL) {
        Person *p = (Person *)data;
        count++;
        id_sum += p->id;
        if (p->id >= 1 && p->id <= 3) {
            ids_found[p->id - 1] = 1;
        }
    }
    
    TEST_ASSERT(count == 3, "Incoming iterator found all 3 predecessors");
    TEST_ASSERT(id_sum == 6, "Incoming iterator found correct predecessors (1+2+3)");
    TEST_ASSERT(ids_found[0] && ids_found[1] && ids_found[2], 
                "Incoming iterator found all expected IDs");
    
    // Test vertex with no incoming neighbors
    adjl_in_iter_init(graph, v5, &iit);
    count = 0;
    while ((data = adjl_in_iter_next(&iit)) != NULL) {
        count++;
    }
    TEST_ASSERT(count == 0, "Vertex with no incoming neighbors returns empty iteration");
    
    // Test vertex with both incoming and outgoing (should only return incoming)
    adjl_graph_add_arc(graph, v4, v5);
    adjl_in_iter_init(graph, v4, &iit);
    count = 0;
    id_sum = 0;
    while ((data = adjl_in_iter_next(&iit)) != NULL) {
        Person *p = (Person *)data;
        count++;
        id_sum += p->id;
    }
    TEST_ASSERT(count == 3, "Incoming iterator ignores outgoing edges");
    TEST_ASSERT(id_sum == 6, "Incoming iterator still finds correct predecessors");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_bfs_traversal() {
    printf("\n=== Testing BFS Traversal ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    /*
    // Create a simple tree structure for BFS
    //     1
    //    / \
    //   2   3
    //  / \
    // 4   5
    */
    
    Person *p1 = create_person(1, "Root");
    Person *p2 = create_person(2, "Left");
    Person *p3 = create_person(3, "Right");
    Person *p4 = create_person(4, "LeftLeft");
    Person *p5 = create_person(5, "LeftRight");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    adjl_graph_add_vertex(graph, p5);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""}, key5 = {5, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    struct adjl_vertex *v5 = adjl_graph_search(graph, &key5);
    
    adjl_graph_add_arc(graph, v1, v2);
    adjl_graph_add_arc(graph, v1, v3);
    adjl_graph_add_arc(graph, v2, v4);
    adjl_graph_add_arc(graph, v2, v5);
    
    TraversalContext ctx = {0};
    adjl_graph_bfs(graph, &key1, &ctx, count_handler);
    
    TEST_ASSERT(ctx.count == 5, "BFS visited all 5 vertices");
    // BFS should visit level by level: 1, then 2 and 3, then 4 and 5
    TEST_ASSERT(ctx.ids[0] == 1, "BFS visited root first");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_dfs_traversal() {
    printf("\n=== Testing DFS Traversal ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    // Same structure as BFS test
    Person *p1 = create_person(1, "Root");
    Person *p2 = create_person(2, "Left");
    Person *p3 = create_person(3, "Right");
    Person *p4 = create_person(4, "LeftLeft");
    Person *p5 = create_person(5, "LeftRight");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    adjl_graph_add_vertex(graph, p5);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""}, key5 = {5, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    struct adjl_vertex *v5 = adjl_graph_search(graph, &key5);
    
    adjl_graph_add_arc(graph, v1, v2);
    adjl_graph_add_arc(graph, v1, v3);
    adjl_graph_add_arc(graph, v2, v4);
    adjl_graph_add_arc(graph, v2, v5);
    
    TraversalContext ctx = {0};
    adjl_graph_dfs(graph, &key1, &ctx, count_handler);
    
    TEST_ASSERT(ctx.count == 5, "DFS visited all 5 vertices");
    TEST_ASSERT(ctx.ids[0] == 1, "DFS visited root first");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_removal_basic() {
    printf("\n=== Testing Vertex Removal - Basic Cases ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    
    // Test 1: Remove isolated vertex (no incoming or outgoing edges)
    Person key3 = {3, ""};
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    void *removed = adjl_graph_remove_vertex(graph, v3);
    
    TEST_ASSERT(removed != NULL, "Remove isolated vertex succeeds");
    TEST_ASSERT(((Person *)removed)->id == 3, "Removed vertex data is correct");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 2, "Vertex count after removal");
    
    Person search_key = {3, ""};
    TEST_ASSERT(adjl_graph_search(graph, &search_key) == NULL, 
                "Removed vertex not found in graph");
    free(removed);
    
    // Test 2: Try to remove vertex with outgoing edges (should fail)
    Person key1 = {1, ""}, key2 = {2, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    adjl_graph_add_arc(graph, v1, v2);
    
    removed = adjl_graph_remove_vertex(graph, v1);
    TEST_ASSERT(removed == NULL, "Remove vertex with outgoing edges fails");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 2, "Vertex count unchanged");
    
    // Test 3: Try to remove vertex with incoming edges (should fail)
    removed = adjl_graph_remove_vertex(graph, v2);
    TEST_ASSERT(removed == NULL, "Remove vertex with incoming edges fails");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 2, "Vertex count unchanged");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_removal_with_edge_cleanup() {
    printf("\n=== Testing Vertex Removal - With Edge Cleanup ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    /*
     * Create graph:  1 -> 2 -> 3
     *                     ↓
     *                     4
     */
    
    Person *p1 = create_person(1, "Alice");
    Person *p2 = create_person(2, "Bob");
    Person *p3 = create_person(3, "Charlie");
    Person *p4 = create_person(4, "Dave");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    
    adjl_graph_add_arc(graph, v1, v2);
    adjl_graph_add_arc(graph, v2, v3);
    adjl_graph_add_arc(graph, v2, v4);
    
    TEST_ASSERT(adjl_graph_edge_count(graph) == 3, "Initial edge count is 3");
    TEST_ASSERT(adjl_vertex_indegree(v2) == 1, "v2 has 1 incoming edge initially");
    TEST_ASSERT(adjl_vertex_outdegree(v2) == 2, "v2 has 2 outgoing edges initially");
    
    // Try to remove v2 (has both incoming and outgoing - should fail)
    void *removed = adjl_graph_remove_vertex(graph, v2);
    TEST_ASSERT(removed == NULL, "Cannot remove v2 with edges");
    
    // Remove all incoming edges to v2 using incoming iterator
    adjl_in_iter iit;
    adjl_in_iter_init(graph, v2, &iit);
    void *pred_data;
    while ((pred_data = adjl_in_iter_next(&iit)) != NULL) {
        Person *pred = (Person *)pred_data;
        Person pred_key = {pred->id, ""};
        struct adjl_vertex *pred_v = adjl_graph_search(graph, &pred_key);
        adjl_graph_remove_arc(graph, pred_v, v2);
    }
    
    TEST_ASSERT(adjl_vertex_indegree(v2) == 0, "v2 has no incoming edges after cleanup");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 2, "Edge count decreased by 1");
    
    // Still cannot remove because of outgoing edges
    removed = adjl_graph_remove_vertex(graph, v2);
    TEST_ASSERT(removed == NULL, "Cannot remove v2 with outgoing edges");
    
    // Remove all outgoing edges from v2 using outgoing iterator
    adjl_out_iter oit;
    adjl_out_iter_init(v2, &oit);
    void *succ_data;
    while ((succ_data = adjl_out_iter_next(&oit)) != NULL) {
        Person *succ = (Person *)succ_data;
        Person succ_key = {succ->id, ""};
        struct adjl_vertex *succ_v = adjl_graph_search(graph, &succ_key);
        adjl_graph_remove_arc(graph, v2, succ_v);
    }
    
    TEST_ASSERT(adjl_vertex_outdegree(v2) == 0, "v2 has no outgoing edges after cleanup");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 0, "All edges removed");
    
    // Now removal should succeed
    removed = adjl_graph_remove_vertex(graph, v2);
    TEST_ASSERT(removed != NULL, "Successfully removed v2 after edge cleanup");
    TEST_ASSERT(((Person *)removed)->id == 2, "Removed correct vertex");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 3, "Vertex count decreased");
    
    free(removed);
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_removal_complex() {
    printf("\n=== Testing Vertex Removal - Complex Scenarios ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    /*
     * Create graph with cycles and multiple paths:
     *     1 -> 2 -> 3
     *     ↓    ↓    ↑
     *     4 -> 5 ---+
     */
    
    Person *p1 = create_person(1, "V1");
    Person *p2 = create_person(2, "V2");
    Person *p3 = create_person(3, "V3");
    Person *p4 = create_person(4, "V4");
    Person *p5 = create_person(5, "V5");
    
    adjl_graph_add_vertex(graph, p1);
    adjl_graph_add_vertex(graph, p2);
    adjl_graph_add_vertex(graph, p3);
    adjl_graph_add_vertex(graph, p4);
    adjl_graph_add_vertex(graph, p5);
    
    Person key1 = {1, ""}, key2 = {2, ""}, key3 = {3, ""}, key4 = {4, ""}, key5 = {5, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    struct adjl_vertex *v2 = adjl_graph_search(graph, &key2);
    struct adjl_vertex *v3 = adjl_graph_search(graph, &key3);
    struct adjl_vertex *v4 = adjl_graph_search(graph, &key4);
    struct adjl_vertex *v5 = adjl_graph_search(graph, &key5);
    
    adjl_graph_add_arc(graph, v1, v2);
    adjl_graph_add_arc(graph, v1, v4);
    adjl_graph_add_arc(graph, v2, v3);
    adjl_graph_add_arc(graph, v2, v5);
    adjl_graph_add_arc(graph, v4, v5);
    adjl_graph_add_arc(graph, v5, v3);
    
    TEST_ASSERT(adjl_graph_edge_count(graph) == 6, "Initial edge count is 6");
    
    // Test: Try to remove v3 (has 2 incoming edges)
    TEST_ASSERT(adjl_vertex_indegree(v3) == 2, "v3 has 2 incoming edges");
    void *removed = adjl_graph_remove_vertex(graph, v3);
    TEST_ASSERT(removed == NULL, "Cannot remove v3 with incoming edges");
    
    // Remove all edges pointing to v3
    int edges_removed = 0;
    adjl_in_iter iit;
    adjl_in_iter_init(graph, v3, &iit);
    void *pred_data;
    
    // Collect predecessors first to avoid iterator invalidation
    int pred_ids[10];
    int pred_count = 0;
    while ((pred_data = adjl_in_iter_next(&iit)) != NULL) {
        Person *pred = (Person *)pred_data;
        pred_ids[pred_count++] = pred->id;
    }
    
    // Now remove the arcs
    for (int i = 0; i < pred_count; i++) {
        Person pred_key = {pred_ids[i], ""};
        struct adjl_vertex *pred_v = adjl_graph_search(graph, &pred_key);
        adjl_graph_remove_arc(graph, pred_v, v3);
        edges_removed++;
    }
    
    TEST_ASSERT(edges_removed == 2, "Removed 2 incoming edges to v3");
    TEST_ASSERT(adjl_vertex_indegree(v3) == 0, "v3 has no incoming edges after cleanup");
    
    // Now v3 should be removable
    removed = adjl_graph_remove_vertex(graph, v3);
    TEST_ASSERT(removed != NULL, "Successfully removed v3 after edge cleanup");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 4, "Vertex count decreased to 4");
    free(removed);
    
    // Test: Remove v5 which still has connections
    TEST_ASSERT(adjl_vertex_indegree(v5) == 2, "v5 has 2 incoming edges");
    TEST_ASSERT(adjl_vertex_outdegree(v5) == 0, "v5 has 0 outgoing edges (was to v3)");
    
    // Remove incoming edges to v5
    adjl_in_iter_init(graph, v5, &iit);
    pred_count = 0;
    while ((pred_data = adjl_in_iter_next(&iit)) != NULL) {
        Person *pred = (Person *)pred_data;
        pred_ids[pred_count++] = pred->id;
    }
    
    for (int i = 0; i < pred_count; i++) {
        Person pred_key = {pred_ids[i], ""};
        struct adjl_vertex *pred_v = adjl_graph_search(graph, &pred_key);
        adjl_graph_remove_arc(graph, pred_v, v5);
    }
    
    removed = adjl_graph_remove_vertex(graph, v5);
    TEST_ASSERT(removed != NULL, "Successfully removed v5");
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 3, "Vertex count decreased to 3");
    free(removed);
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_vertex_flagging() {
    printf("\n=== Testing Vertex Flagging ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    Person *p1 = create_person(1, "Alice");
    adjl_graph_add_vertex(graph, p1);
    
    Person key1 = {1, ""};
    struct adjl_vertex *v1 = adjl_graph_search(graph, &key1);
    
    // Test that flagging doesn't crash (no return value to test)
    adjl_vertex_flag(v1, 42);
    adjl_vertex_flag(v1, 0);
    adjl_vertex_flag(v1, -1);
    
    TEST_ASSERT(1, "Vertex flagging operations complete without error");
    
    adjl_graph_destroy(graph, &person_oc);
}

void test_complex_graph() {
    printf("\n=== Testing Complex Graph ===\n");
    
    struct adjl_graph *graph = adjl_graph_create(person_compare);
    
    // Create a more complex graph
    const int num_vertices = 10;
    Person *people[10];
    
    for (int i = 0; i < num_vertices; i++) {
        char name[32];
        sprintf(name, "Person%d", i);
        people[i] = create_person(i, name);
        adjl_graph_add_vertex(graph, people[i]);
    }
    
    // Add various connections
    Person keys[10];
    struct adjl_vertex *vertices[10];
    for (int i = 0; i < num_vertices; i++) {
        keys[i].id = i;
        vertices[i] = adjl_graph_search(graph, &keys[i]);
    }
    
    // Create a connected graph with cycles
    adjl_graph_add_arc(graph, vertices[0], vertices[1]);
    adjl_graph_add_arc(graph, vertices[0], vertices[2]);
    adjl_graph_add_arc(graph, vertices[1], vertices[3]);
    adjl_graph_add_arc(graph, vertices[2], vertices[3]);
    adjl_graph_add_arc(graph, vertices[3], vertices[4]);
    adjl_graph_add_arc(graph, vertices[4], vertices[1]); // Create cycle
    adjl_graph_add_arc(graph, vertices[5], vertices[6]);
    adjl_graph_add_arc(graph, vertices[6], vertices[7]);
    adjl_graph_add_arc(graph, vertices[7], vertices[8]);
    adjl_graph_add_arc(graph, vertices[8], vertices[9]);
    
    TEST_ASSERT(adjl_graph_vertex_count(graph) == 10, "Complex graph has 10 vertices");
    TEST_ASSERT(adjl_graph_edge_count(graph) == 10, "Complex graph has 10 edges");
    
    // Test iteration through all vertices
    adjl_vertex_iterator it;
    adjl_v_iter_init(graph, &it);
    int count = 0;
    while (adjl_v_iter_next(&it) != NULL) {
        count++;
    }
    TEST_ASSERT(count == 10, "Iteration covers all vertices in complex graph");
    
    adjl_graph_destroy(graph, &person_oc);
}

int main() {
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Adjacency List Graph Test Suite          ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    
    test_graph_creation();
    test_vertex_operations();
    test_arc_operations();
    test_vertex_iterator();
    test_outgoing_neighbor_iterator();
    test_incoming_neighbor_iterator();
    test_bfs_traversal();
    test_dfs_traversal();
    test_vertex_removal_basic();
    test_vertex_removal_with_edge_cleanup();
    test_vertex_removal_complex();
    test_vertex_flagging();
    test_complex_graph();
    
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║  Test Results                              ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  Passed: %-4d                              ║\n", tests_passed);
    printf("║  Failed: %-4d                              ║\n", tests_failed);
    printf("║  Total:  %-4d                              ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n🎉 All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed.\n");
        return 1;
    }
}