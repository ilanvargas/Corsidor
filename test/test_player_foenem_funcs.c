#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "move_check.h"
#include "graph.h"
#include "move.h"
#include "player.h"
#include "test.h"

void djikstra_with_prev(struct graph_t* graph,
                        vertex_t source,
                        vertex_t prev_pos,
                        unsigned int* dist,
                        vertex_t op_pos,
                        vertex_t* prev);

int find_wall_candidate_for_edge(struct graph_t* graph, vertex_t source, vertex_t target, struct edge_t candidate[2]);

unsigned int evaluate_wall_gain(struct graph_t* graph, struct edge_t wall[2], vertex_t pos, vertex_t opp_pos, vertex_t opp_prev_pos, vertex_t opp_checkpoint, unsigned int original_dist);

void find_best_wall(struct graph_t* graph, vertex_t pos, vertex_t opp_pos, vertex_t opp_prev_pos, vertex_t opp_checkpoint, struct edge_t best_wall[2], unsigned int* gain_ptr);

void test_dijkstra_with_prev() {
  struct graph_t* graph = graph_init(5, PARALLELOGRAM);
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));
  for (unsigned int i = 0; i < n; i++) {
    prev[i] = UINT_MAX;
  }
  prev[10] = 5; /* previous position before 10 */

  djikstra_with_prev(graph, 10, (vertex_t)INVALID_VERTEX, dist, 14, prev);

  for (unsigned int i = 0; i < n; i++) {
    printf("Distance from 10 to %u: %u\n", i, dist[i]);
  }
  printf("\n");

  free(dist);
  free(prev);
  graph_free(graph);
}

void test_find_wall_candidate_for_edge() {
  struct graph_t* graph = graph_init(5, PARALLELOGRAM);
  struct edge_t candidate[2];
  int result = find_wall_candidate_for_edge(graph, 0, 1, candidate);
  printf("find_wall_candidate_for_edge(0,1): %d\n", result);
  if (result) {
    printf("Candidate: (%d,%d) (%d,%d)\n", candidate[0].fr, candidate[0].to, candidate[1].fr, candidate[1].to);
  }
  graph_free(graph);
}

void test_evaluate_wall_gain() {
  struct graph_t* graph = graph_init(5, PARALLELOGRAM);
  struct edge_t wall[2] = {{0,6}, {0,5}};
  unsigned int gain = evaluate_wall_gain( graph, wall, 14, 10, (vertex_t)INVALID_VERTEX, 0, 2);
  printf("evaluate_wall_gain: %u\n", gain);
  graph_free(graph);
}

void test_find_best_wall() {
  struct graph_t* graph = graph_init(5, PARALLELOGRAM);
  struct edge_t best_wall[2];
  unsigned int gain = 0;
  find_best_wall(graph, 10, 14, (vertex_t)INVALID_VERTEX, 4, best_wall, &gain);
  printf("find_best_wall gain: %u\n", gain);
  if (gain > 0) {
    printf("Best wall: (%d,%d) (%d,%d)\n", best_wall[0].fr, best_wall[0].to, best_wall[1].fr, best_wall[1].to);
  }
  graph_free(graph);
}

int main() {
  RUN(test_dijkstra_with_prev())
  RUN(test_find_wall_candidate_for_edge())
  RUN(test_evaluate_wall_gain())
  RUN(test_find_best_wall())

  return EXIT_SUCCESS;
}
