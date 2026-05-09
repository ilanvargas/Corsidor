#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph_impl.h"
#include "player.h"
#include "test.h"

void djikstra_with_prev(struct graph_t* graph,
                        vertex_t source,
                        unsigned int* dist,
                        vertex_t op_pos,
                        vertex_t* prev);

void test_dijkstra_with_prev() {
  struct graph_t* graph = graph_init(5, PARALLELOGRAM);
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));
  for (unsigned int i = 0; i < n; i++) {
    prev[i] = UINT_MAX;
  }
  prev[10] = 5; /* previous position before 10 */

  djikstra_with_prev(graph, 10, dist, 14, prev);

  for (unsigned int i = 0; i < n; i++) {
    printf("Distance from 10 to %u: %u\n", i, dist[i]);
  }
  printf("\n");

  free(dist);
  free(prev);
  graph_free(graph);
}
/*
void test_dijkstra_with_prev_sixty_degree() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, 10, dist, 14, prev);

  printf("\t 60-degree acceleration: ");
  CMP_INT(dist[10], 1)
  CMP_INT(dist[30], 2)
  printf("\n");

  free(dist);
  free(prev);
  graph_free(graph);
}
*/

int main() {
  RUN(test_dijkstra_with_prev())
  // RUN(test_dijkstra_with_prev_sixty_degree())
  return EXIT_SUCCESS;
}
