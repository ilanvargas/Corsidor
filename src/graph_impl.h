#ifndef __GRAPH_IMPL_H__
#define __GRAPH_IMPL_H__

#include "graph.h"

#define IS_ADJACENT(graph, u, v) \
  (gsl_spmatrix_uint_get((graph)->t, (u), (v)) != NO_EDGE)

#define IS_VERTEX_VALID(graph, v) ((v) < (graph)->t->size1)

#define IS_WALL(val) (((val) & WALL_MASK) != NO_EDGE)

/* Initialisation of the game graph
 * PARAM:
 * - m:     dictates the graph size
 * - type:  the graph type (according to graph.h)
 * RETURN:
 * - a pointer to struct graph_t carring the game graph
 */
struct graph_t* graph_init(size_t m, enum graph_type_t type);

/** Find dir from differences in coordinates i,j of 2 points
 */
enum dir_t translate_dir(int di, int dj);

/** Free struct graph_t graph
 */
void graph_free(struct graph_t* graph);

/** Gives to a pointer to edge
 * Usefull to modify it (add wall)
 */
unsigned int* get_edge(gsl_spmatrix_uint* csr, vertex_t fr, vertex_t to);

/** Build wall in graph on edges e
 *  WALL_MASK on edges e
 *  PRECOND: The wall is valid
 */
void build_wall(struct graph_t* graph, struct edge_t e[2]);

/* Generate a .dot file from a struct graph_t for graphical representation
 * The dot file will be located in the working directory
 */
void graph_show(struct graph_t* graph, char* file_name);

#endif  // __GRAPH_IMPL_H__
