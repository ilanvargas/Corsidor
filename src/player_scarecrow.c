#include <stdlib.h>

#include "player.h"

unsigned int p_id;
struct graph_t* p_graph;
vertex_t p_pos;
vertex_t op_pos;

int* is_visited;

char const* get_player_name() {
  return "Scarecrow";
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;
  p_graph = graph;
  p_pos = p_graph->start[p_id];
  op_pos = p_graph->start[1 - p_id];

  is_visited = calloc(p_graph->t->size1, sizeof(int));
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  gsl_spmatrix_uint* csr = p_graph->t;

  if (previous_moves[1 - p_id].t == MOVE || previous_moves[1 - p_id].t == JUMP)
    op_pos = previous_moves[1 - p_id].m;

  if (previous_moves[p_id].t == WATERLOO)
    p_pos = previous_moves[p_id].m;

  vertex_t u = p_pos;
  for (int k = csr->p[p_pos]; k < csr->p[p_pos + 1]; k++) {
    vertex_t v = csr->i[k];

    if (op_pos != v) {
      u = v;
      if (!is_visited[v]) {
        is_visited[v] = (v != p_graph->start[p_id]) ? 1 : 0;
        p_pos = v;
        return (struct move_t){.c = p_id, .t = MOVE, .m = v};
      }
    }
  }

  // fallback : last neighbour  
  p_pos = u;
  is_visited[u] = (u != p_graph->start[p_id]) ? 1 : 0;

  return (struct move_t){.c = p_id, .t = MOVE, .m = u};
}

void finalize() {
  free(is_visited);

  // free graph
  gsl_spmatrix_uint_free(p_graph->t);
  free(p_graph->objectives);
  free(p_graph);

  return;
}
