#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "move_check.h"
#include "player.h"

int path_dir_diff(struct graph_t* graph,
                  vertex_t prev,
                  vertex_t fr,
                  vertex_t to,
                  enum dir_t* dir_go);
int is_straigh_clear_path(struct graph_t* graph,
                          vertex_t to,
                          vertex_t fr,
                          vertex_t op,
                          enum dir_t dir_go,
                          int max_dist);

char const* p_name = "Foenem";
unsigned int p_id;
struct graph_t p_graph;
vertex_t p_pos;
vertex_t p_prev_pos;
vertex_t op_pos;
int p_walls_left;

char const* get_player_name() { return p_name; }

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;
  p_graph = *graph;
  p_pos = p_graph.start[p_id];
  p_prev_pos = INVALID_VERTEX;
  op_pos = p_graph.start[1 - p_id];
  p_walls_left = graph->num_edges / 16;
}

static void djikstra_with_prev(struct graph_t* graph,
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

unsigned int* djikstra(struct graph_t* graph, vertex_t source) {
  unsigned int n = graph->num_vertices;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, source, dist, prev);
  free(prev);
  return dist;
}

static int objective_reachable_with_acceleration(vertex_t pos,
                                                 vertex_t objective,
                                                 vertex_t prev_pos,
                                                 vertex_t op_pos) {
  enum dir_t dir_go;
  int dir_diff = path_dir_diff(&p_graph, prev_pos, pos, objective, &dir_go);

  if (dir_diff == 0) {
    return is_straigh_clear_path(&p_graph, objective, pos, op_pos, dir_go, MAX_DIST);
  }
  if (dir_diff == 1) {
    return is_straigh_clear_path(&p_graph, objective, pos, op_pos, dir_go, MAX_DIST_60_DEG);
  }
  return 0;
}

static unsigned int get_closest_objective(vertex_t pos,
                                            vertex_t* next_objective) {
  unsigned int* dist = djikstra(&p_graph, pos);
  unsigned int best_dist = UINT_MAX;
  vertex_t best_objective = p_graph.objectives[0];

  for (unsigned int i = 0; i < p_graph.num_objectives; i++) {
    vertex_t objective = p_graph.objectives[i];
    unsigned int effective_dist = dist[objective];
    if (objective_reachable_with_acceleration(pos, objective, p_prev_pos, op_pos)) {
      effective_dist = 1;
    }
    if (effective_dist < best_dist) {
      best_dist = effective_dist;
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
/* Build the shortest path from source to target 
* Returns 1 if a path exists, 0 otherwise. If a path exists, it is stored in the
* path array and its length is stored in path_len.
*/
static int build_shortest_path(vertex_t source,
                               vertex_t target,
                               vertex_t* path,
                               unsigned int* path_len) {
  unsigned int n = p_graph.num_vertices;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));
  
  djikstra_with_prev(&p_graph, source, dist, prev);

  
  if (dist[target] == UINT_MAX) { // No path exists (should not happen in a valid game state)
    free(dist);
    free(prev);
    *path_len = 0;
    return 0;
  }

  unsigned int len = 0;
  vertex_t cur = target;
  while (cur != UINT_MAX) { // Trace back from target to source using prev array
    path[len] = cur;
    if (cur == source) {
      break;
    }
    len++;
    cur = prev[cur];
  }

  for (unsigned int i = 0; i < len / 2; i++) { // Reverse the path to get it from source to target
    vertex_t tmp = path[i];
    path[i] = path[len - 1 - i];
    path[len - 1 - i] = tmp;
  }

  free(dist);
  free(prev);

  *path_len = len;
  return 1;
}

/* Set or clear the wall bit for a given edge in the graph */
static void set_wall_mask(struct graph_t* graph,
                          vertex_t fr,
                          vertex_t to,
                          int enable) {
  unsigned int value = gsl_spmatrix_uint_get(graph->t, fr, to);
  if (enable) {
    value |= WALL_MASK; // Set the wall bit
  } else {
    value &= ~WALL_MASK; // Clear the wall bit
  }
  gsl_spmatrix_uint_set(graph->t, fr, to, value);
}

/* Evaluate the gain from placing walls at the specified edges 
* Returns the increase in distance for the opponent to reach their checkpoint, or 0 if there is no increase,
* or UINT_MAX if it becomes impossible for the opponent to reach their checkpoint.
*/
static unsigned int evaluate_wall_gain(struct edge_t wall[2],
                                       vertex_t opp_pos,
                                       vertex_t opp_checkpoint,
                                       unsigned int original_dist) {

  set_wall_mask(&p_graph, wall[0].fr, wall[0].to, 1);
  set_wall_mask(&p_graph, wall[0].to, wall[0].fr, 1);
  set_wall_mask(&p_graph, wall[1].fr, wall[1].to, 1);
  set_wall_mask(&p_graph, wall[1].to, wall[1].fr, 1);

  unsigned int* dist = dijkstra(&p_graph, opp_pos);
  unsigned int new_dist = dist[opp_checkpoint];
  free(dist);

  set_wall_mask(&p_graph, wall[0].fr, wall[0].to, 0);
  set_wall_mask(&p_graph, wall[0].to, wall[0].fr, 0);
  set_wall_mask(&p_graph, wall[1].fr, wall[1].to, 0);
  set_wall_mask(&p_graph, wall[1].to, wall[1].fr, 0);

  if (original_dist == UINT_MAX || new_dist == UINT_MAX) {
    return (new_dist == UINT_MAX && original_dist != UINT_MAX) ? UINT_MAX : 0;
  }
  return (new_dist > original_dist) ? new_dist - original_dist : 0;
}

void finalize() { return; }

