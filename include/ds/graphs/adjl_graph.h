#ifndef GRAPHS_ADJL_GRAPH_H
#define GRAPHS_ADJL_GRAPH_H

#include "../../concepts/include/object_concept.h"
#include "../../concepts/include/allocator_concept.h"
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Directed graph with adjacency list.
 * 
 * @brief Graph ADT.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct adjlgraph *adjlg` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal nodes are owned by the underlying list and managed by system (malloc/free) for now,
 * - void *references to data are entirely owned by user. adjlist_deinit might be helpful to destruct remaining
 * - objects in the graph.
 * @{
 */

struct clist_item;
struct adjl_vertex;
struct adjl_arc;
struct adjl_graph;

/**
 * @struct adjl_vertex_iterator
 * @brief Opaque iterator for graph vertices. 
 * Allocated on the stack by the user, managed by the API.
 */
typedef struct {
    struct adjl_graph *graph;       // Reference to container (for sentinel check)
    struct clist_item *current;     // Current position
} adjl_vertex_iterator;

/**
 * @struct adjl_neighbor_iterator
 * @brief Opaque iterator for a vertex's outgoing edges.
 */
typedef struct {
    struct adjl_arc   *current_arc; // Current position (void* wrapper in implementation)
} adjl_neighbor_iterator;

/**
 * @name Create & Destroy
 * @{
 */

/**
 * @brief Creates graph.
 * @param cmp Function pointer to compare data stored in the vertices.
 * @return graph, NULL if fails.
 */
struct adjl_graph *adjl_graph_create(int (*cmp) (const void *key, const void *data));

/**
 * @brief Destoys and frees graph.
 * @param[in, out] gr Pointer to the graph instance.
 * @param[in] oc Pointer to an object_concept used to init/deinit or free
 * nodes and its contents.
 */
void adjl_graph_destroy(struct adjl_graph *gr, struct object_concept *oc);

/** @} */

/**
 * @name Insertion & Removal
 * @{
 */

/**
 * @brief Adds a vertex into the graph.
 * @param[in] new_data new object to store in the graph.
 * @return 0 on success, non-zero otherwise.
 * 
 * @warning @p new_data cannot be NULL, otherwise the assertion will fail.
 */
int adjl_graph_add_vertex(struct adjl_graph *gr, void *new_data);

/**
 * @brief Removes a vertex from the graph (usually get from a search function)
 * @param[in] v vertex requested to be removed.
 * @return void * reference that was stored in the vertex, NULL if it fails.
 * Note that the removal will fail if only v is connected to another vertex.
 */
void *adjl_graph_remove_vertex(struct adjl_graph *gr, struct adjl_vertex *v);

/**
 * @brief Adds an arc into the graph.
 * @param[in] v1 Source vertex.
 * @param[in] v2 Destination vertex.
 * @return 0 on success, non-zero otherwise.
 */
int adjl_graph_add_arc(struct adjl_graph *gr, struct adjl_vertex *v1, struct adjl_vertex *v2);

/**
 * @brief Removes an arc from the graph.
 * @param[in] src Source vertex.
 * @param[in] dest Destination vertex.
 * @return 0 on success, non-zero otherwise.
 */
int adjl_graph_remove_arc(struct adjl_graph *gr, struct adjl_vertex *src, struct adjl_vertex *dest);

/** @} */

/**
 * @name Vertex API
 * @{
 */

/**
 * @brief Flags the vertex, which might be useful in your
 * traversal logic.
 * @param[in] flag AN integer value for specific state.
 */
void adjl_vertex_flag(struct adjl_vertex *v, int flag);

/**
 * @return indegree of the vertex.
 */
size_t adjl_vertex_indegree(const struct adjl_vertex *v);

/**
 * @return outdegree of the vertex.
 */
size_t adjl_vertex_outdegree(const struct adjl_vertex *v);

/**
 * @brief Retrieves the user data from an opaque vertex handle.
 */
void *adjl_vertex_get_data(struct adjl_vertex *v);

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * @brief Initializes a vertex iterator to the start of the graph.
 * @param it Pointer to user-allocated iterator struct.
 */
void adjl_v_iter_init(struct adjl_graph *gr, adjl_vertex_iterator *it);

/**
 * @brief Returns current vertex data and advances iterator.
 * @return void* pointer to user data, NULL if there is not.
 */
void *adjl_v_iter_next(adjl_vertex_iterator *it);

/**
 * @brief Initializes a neighbor iterator for a specific vertex.
 */
void adjl_n_iter_init(struct adjl_vertex *v, adjl_neighbor_iterator *it);

/**
 * @brief Returns data of the DESTINATION vertex and advances.
 * @return void* pointer to neighbor's data, NULL if there is not.
 */
void *adjl_n_iter_next(adjl_neighbor_iterator *it);

/** @} */

/**
 * @name Retrieval
 * @{
 */

/**
 * @brief Searches a vertex that contains data matching @p key.
 * @param[in] key key to be searched.
 * @return vertex
 */
struct adjl_vertex *adjl_graph_search(struct adjl_graph *gr, const void *key);

/** @} */

/**
 * @name Traversal
 * @{
 */

/**
 * @brief Traverses the graph in Breadth-First order.
 * @param[in] context pointer for ease to use in handler.
 * @param[in] handler Function pointer that executes on data
 * stored in the vertex and context pointer.
 */
void adjl_graph_bfs(struct adjl_graph *gr, void *start_key, void *context, void (*handler) (void *item, void *context));

/**
 * @brief Traverses the graph in Depth-First order.
 * @param[in] context pointer for ease to use in handler.
 * @param[in] handler Function pointer that executes on data
 * stored in the vertex and context pointer.
 */
void adjl_graph_dfs(struct adjl_graph *gr, void *start_key, void *context, void (*handler) (void *item, void *context));

/** @} */

/**
 * @name Inspection
 * @{
 */

/**
 * @return 1 if empty, 0 if not.
 */
int adjl_graph_empty(const struct adjl_graph *gr);

/**
 * @return Vertex count in the graph.
 */
size_t adjl_graph_vertex_count(const struct adjl_graph *gr);

/**
 * @return Arc count in the graph.
 */
size_t adjl_graph_edge_count(const struct adjl_graph *gr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // GRAPHS_ADJL_GRAPH_H