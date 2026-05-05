#include <stdlib.h>

#include "player.h"

#define IS_ADJACENT(graph, u, v) \
  (gsl_spmatrix_uint_get((graph)->t, (u), (v)) != NO_EDGE)

char const* p_name = "Scarecrow";
unsigned int p_id;
struct graph_t* p_graph;
vertex_t p_pos;
vertex_t op_pos;

int* is_visited;

char const* get_player_name() {
  return p_name;
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;

  // copy graph
  p_graph = malloc(sizeof(struct graph_t));
  p_graph->type = graph->type;
  p_graph->num_vertices = graph->num_vertices;
  p_graph->num_edges = graph->num_edges;
  p_graph->num_objectives = graph->num_objectives;
  p_graph->t = gsl_spmatrix_uint_alloc(graph->t->size1, graph->t->size2);
  gsl_spmatrix_uint_memcpy(p_graph->t, graph->t);
  p_graph->objectives = malloc(graph->num_objectives * sizeof(vertex_t));
  for (unsigned int i = 0; i < graph->num_objectives; i++)
      p_graph->objectives[i] = graph->objectives[i];
  for (unsigned int i = 0; i < NUM_PLAYERS; i++)
      p_graph->start[i] = graph->start[i];

  p_pos = p_graph->start[p_id];
  op_pos = p_graph->start[1 - p_id];
  is_visited = calloc(p_graph->t->size1, sizeof(int));
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  if (previous_moves[1 - p_id].t == MOVE || previous_moves[1 - p_id].t == JUMP)
    op_pos = previous_moves[1 - p_id].m;

  if (previous_moves[p_id].t == WATERLOO)
    p_pos = previous_moves[p_id].m;

  unsigned int u = 0;
  for (unsigned int v = 0; v < p_graph->t->size1; v++) {
    if (IS_ADJACENT(p_graph, p_pos, v) && op_pos != v) {
      u = v;
      if (is_visited[v] == 0) {
        is_visited[v] = 1;
        p_pos = v;
        return (struct move_t){.c = p_id, .t = MOVE, .m = v};
      }
    }
  }

  p_pos = u;
  return (struct move_t){.c = p_id, .t = MOVE, .m = u};
}

void finalize() {
  free(is_visited);

  gsl_spmatrix_uint_free(p_graph->t);
  free(p_graph->objectives);
  free(p_graph);

  return;
}
