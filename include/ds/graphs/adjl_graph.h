#ifndef GRAPHS_ADJL_GRAPH_H
#define GRAPHS_ADJL_GRAPH_H

#include <ds/utils/object_concept.h>
#include <ds/utils/allocator_concept.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file adjl_graph.h
 * @brief Defines the interface for an unweighted directed graph (Adjacency List).
 */

/**
 * @defgroup ADJL_GRAPH Adjacency List Graph
 * @ingroup GRAPHS
 * @brief Directed graph implementation using linked lists for vertices and edges.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct adjl_graph *` and `struct adjl_vertex *` pointers must be valid (non-NULL).
 * - **Ownership**: 
 * - Internal nodes (`vertex` and `arc` structs) are managed by malloc/free for now.
 * - User Data (`void *`) is owned entirely by the user. 
 * - Use `adjl_graph_destroy` with an `object_concept` to clean up user data automatically.
 * @{
 */

struct clist_item;
struct adjl_vertex;
struct adjl_arc;
struct adjl_graph;

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
 * nodes and its contents. It might be NULL, .deinit method might be NULL,
 * .init method isnt even used. 
 */
void adjl_graph_destroy(struct adjl_graph *gr, struct object_concept *oc);

/** @} End of the Create & Destroy */

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
 * @return The `void *` data that was stored in the vertex, or NULL on failure.
 * @warning **Topological Dependency**: This operation fails (returns NULL) if 
 * the vertex still has incoming or outgoing edges. You must remove arcs first.
 * You can use neighbor iterators such as @ref adjl_out_iter and @ref adjl_in_iter.
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

/** @} End of the Insertion & Removal */

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

/** @return indegree of the vertex. */
size_t adjl_vertex_indegree(const struct adjl_vertex *v);

/** @return outdegree of the vertex. */
size_t adjl_vertex_outdegree(const struct adjl_vertex *v);

/** @brief Retrieves the user data from an opaque vertex handle. */
void *adjl_vertex_get_data(struct adjl_vertex *v);

/** @} End of the Vertex API */

/**
 * @name Iterators
 * @{
 */

/**
 * @struct adjl_vertex_iterator
 * @brief Opaque iterator for graph vertices. 
 * Allocated on the stack by the user, managed by the API.
 */
typedef struct {
    struct adjl_graph       *graph;         ///< Pointer to the grah instance.
    struct clist_item       *current;       ///< Current vertex hook.
} adjl_vertex_iterator;

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
 * @struct adjl_out_iter
 * @brief Iterator for a vertex's **outgoing** edges (successors).
 * This iterates over vertices `V` such that `(Current -> V)` exists.
 * @note **Complexity**: This operation is **O(1)** per step.
 */
typedef struct {
    struct adjl_arc         *current_arc;       ///< Current position in the adjacency list.
} adjl_out_iter;

/**
 * @brief Initializes an iterator for outgoing neighbors.
 * @param[in]  v  The source vertex.
 * @param[out] it User-allocated iterator.
 */
void adjl_out_iter_init(const struct adjl_vertex *v, adjl_out_iter *it);

/**
 * @brief Advances to the next outgoing neighbor.
 * @return Data of the **destination** vertex, or NULL if finished.
 */
void *adjl_out_iter_next(adjl_out_iter *it);

/**
 * @struct adjl_in_iter
 * @brief Iterator for a vertex's **incoming** edges (predecessors).
 * This iterates over vertices `V` such that `(V -> Current)` exists.
 * @warning **Performance Alert**: Since this graph is a singly-linked adjacency list,
 * finding incoming edges requires a full graph scan.
 * @note **Complexity**: Completing a full iteration takes **O(V + E)** time. 
 * Do not use this inside tight loops.
 */
typedef struct {
    const struct adjl_graph         *graph;             ///< Pointer to the graph instance for vertex traversal.
    const struct adjl_vertex        *target;            ///< Target vertex to find in adjacency lists.
    struct clist_item               *curr_v_node;       ///< Current vertex in graphs vertex list.
    struct adjl_arc                 *curr_arc;          ///< Current arc in one of the adjacency list of the vertices
} adjl_in_iter;

/**
 * @brief Initializes an iterator for incoming neighbors.
 * @param[in]  gr The graph instance (required for scanning).
 * @param[in]  v  The target vertex (to find who points TO this).
 * @param[out] it User-allocated iterator.
 */
void adjl_in_iter_init(const struct adjl_graph *gr, const struct adjl_vertex *v, adjl_in_iter *it);

/**
 * @brief Scans the graph for the next vertex that points to the target.
 * @return Data of the **source** vertex, or NULL if finished.
 */
void *adjl_in_iter_next(adjl_in_iter *it);

/** @} End of the Iterators */

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

/** @} End of the Retrieval */

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

/** @} End of the Traversal */

/**
 * @name Inspection
 * @{
 */

/** @return 1 if empty, 0 if not. */
int adjl_graph_empty(const struct adjl_graph *gr);

/** @return Vertex count in the graph. */
size_t adjl_graph_vertex_count(const struct adjl_graph *gr);

/** @return Arc count in the graph. */
size_t adjl_graph_edge_count(const struct adjl_graph *gr);

/** @} End of the Inspection */

/** @} End of the ADJL_GRAPH group */

#ifdef __cplusplus
}
#endif

#endif // GRAPHS_ADJL_GRAPH_H