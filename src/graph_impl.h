#ifndef __GRAPH_IMPL_H__
#define __GRAPH_IMPL_H__

#include "graph.h"

/* Initialisation of the game graph
 * PARAM:
 * - m:     dictates the graph size
 * - type:  the graph type (according to graph.h)
 * RETURN:
 * - a struct graph_t carring the game graph
 */
struct graph_t graph_init(size_t m, enum graph_type_t type);

/* Generate a .dot file from a struct graph_t for graphical representation
 * The dot file will be located in the working directory
 */
void graph_show(struct graph_t* graph, char* file_name);

#endif // __GRAPH_IMPL_H__
