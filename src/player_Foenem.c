#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "move_check.h"
#include "player.h"

char const* p_name = "Foenem";
unsigned int p_id;
struct graph_t p_graph;
vertex_t p_pos;
vertex_t op_pos;
int p_walls_left;

char const* get_player_name() {
  return p_name;
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;
  p_graph = *graph;
  p_pos = p_graph.start[p_id];
  op_pos = p_graph.start[1 - p_id];
  p_walls_left = graph->num_edges / 16;
}

static void dijkstra_with_prev(struct graph_t* graph,
                               vertex_t source,
                               unsigned int* dist,
                               vertex_t* prev) {
  unsigned int n = graph->num_vertices;

  for (unsigned int i = 0; i < n; i++) {
    dist[i] = UINT_MAX;
    prev[i] = UINT_MAX;
  }
  dist[source] = 0;

  int* visited = malloc(n * sizeof(int));
  for (unsigned int i = 0; i < n; i++) {
    visited[i] = 0;
  }

  for (unsigned int iter = 0; iter < n; iter++) {
    vertex_t u = UINT_MAX;
    unsigned int best_dist = UINT_MAX;

    for (unsigned int v = 0; v < n; v++) {
      if (!visited[v] && dist[v] < best_dist) {
        u = v;
        best_dist = dist[v];
      }
    }

    if (u == UINT_MAX || best_dist == UINT_MAX) {
      break;
    }

    visited[u] = 1;

    /* Relax edges */
    for (unsigned int v = 0; v < n; v++) {
      if (IS_ADJACENT(graph, u, v) && !IS_WALL(graph, u, v)) {
        if (dist[u] + 1 < dist[v]) {
          dist[v] = dist[u] + 1;
          prev[v] = u;
        }
      }
    }
  }

  free(visited);
}

unsigned int* dijkstra(struct graph_t* graph, vertex_t source) {
  unsigned int n = graph->num_vertices;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  dijkstra_with_prev(graph, source, dist, prev);
  free(prev);
  return dist;
}

static unsigned int get_closest_objective(vertex_t pos,
                                          vertex_t* next_objective) {
  unsigned int* dist = dijkstra(&p_graph, pos);
  unsigned int best_dist = UINT_MAX;
  vertex_t best_objective = p_graph.objectives[0];

  for (unsigned int i = 0; i < p_graph.num_objectives; i++) {
    vertex_t objective = p_graph.objectives[i];
    if (dist[objective] < best_dist) {
      best_dist = dist[objective];
      best_objective = objective;
    }
  }

  free(dist);
  *next_objective = best_objective;
  return best_dist;
}

static unsigned int get_next_objective(vertex_t pos) {
  vertex_t objective = UINT_MAX;
  get_closest_objective(pos, &objective);
  return objective;
}
