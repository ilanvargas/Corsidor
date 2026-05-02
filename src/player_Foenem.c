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
  unsigned int n = graph->t->size1;

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

/*
* Get the distance from source to all vertices using Dijkstra's algorithm.
* Returns an array of distances, where dist[i] is the distance from source to vertex i. 
* If a vertex is unreachable from source, its distance will be UINT_MAX.
*/
unsigned int* djikstra(struct graph_t* graph, vertex_t source) {
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, source, dist, prev);
  free(prev);
  return dist;
}

/*
* Check if an objective is reachable with acceleration.
* Returns 1 if reachable, 0 otherwise.
*/
static int objective_reachable_with_acceleration(vertex_t pos,
                                                 vertex_t objective,
                                                 vertex_t prev_pos,
                                                 vertex_t op_pos) {
  enum dir_t dir_go;
  if (IS_ADJACENT(&p_graph, pos, objective) && !IS_WALL(&p_graph, pos, objective)) {
    return 1;
  }
                                                
  int dir_diff = path_dir_diff(&p_graph, prev_pos, pos, objective, &dir_go);
                                        
  if (dir_diff == 0) {
    return is_straigh_clear_path(&p_graph, objective, pos, op_pos, dir_go, MAX_DIST);
  }
  if (dir_diff == 1) {
    return is_straigh_clear_path(&p_graph, objective, pos, op_pos, dir_go, MAX_DIST_60_DEG);
  }
  return 0;
}

/*
* Get the closest objective to the current position, and its distance.
* returns the distance, and sets *next_objective to the closest objective.
*/
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
      if (len == 0) { // Source and target are the same
        free(dist);
        free(prev);
        *path_len = 0;
        return 1;
      }
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

/*
  * Find a pair of edges that can be used to place a wall between source and target
  * Returns 1 if such a pair of edges is found, 0 otherwise. 
  * If found, the candidate edges are stored in the candidate array.
 */
static int find_wall_candidate_for_edge(vertex_t source,
                                        vertex_t target,
                                        struct edge_t candidate[2]) {
  unsigned int n = p_graph.t->size1;

  for (unsigned int other = 0; other < n; other++) {
    if (other == target) {
      continue;
    }
    if (!IS_ADJACENT(&p_graph, source, other) ||
        !IS_ADJACENT(&p_graph, target, other)) { // other must be adjacent to both source and target
      continue;
    }

    if (IS_WALL(&p_graph, source, other) || IS_WALL(&p_graph, source, target)) {
      continue;
    }

    candidate[0].fr = source;
    candidate[0].to = target;
    candidate[1].fr = source;
    candidate[1].to = other;
    return 1;
  }

  return 0;
}

/*
* Find the best wall to place to increase the opponent's distance to their closest objective.
* best_wall will contain the edges to place the wall on, and gain_ptr will contain the gain in distance for the opponent.
*/
static void find_best_wall(vertex_t opp_pos,
                           vertex_t opp_checkpoint,
                           struct edge_t best_wall[2],
                           unsigned int* gain_ptr) {
  unsigned int n = p_graph.t->size1;
  vertex_t path[n];
  unsigned int path_len = 0;
  unsigned int* opp_dist = dijkstra(&p_graph, opp_pos);
  unsigned int original_dist = opp_dist[opp_checkpoint];
  free(opp_dist);

  *gain_ptr = 0;
  best_wall[0].fr = UINT_MAX;
  best_wall[0].to = UINT_MAX;
  best_wall[1].fr = UINT_MAX;
  best_wall[1].to = UINT_MAX;

  int path_exists = build_shortest_path(opp_pos, opp_checkpoint, path, &path_len);
  if (!path_exists) { // No path exists, no wall can be placed to increase distance
    return;
  }
  if (path_len == 0) { // Opp is already at the checkpoint, 
      return;
    }
  if (path_len == 1) { // Opp and opp_checkpoint are adjacent,  
                       // so we consider it as the best wall with maximum gain
  unsigned int gain = evaluate_wall_gain(candidate, opp_pos,
                    opp_checkpoint, original_dist);  
  if (gain != UINT_MAX) {
    *gain_ptr = gain;
    find_wall_candidate_for_edge(opp_pos, opp_checkpoint, best_wall);
    return;
  } else { // If gain is UINT_MAX, it means placing the wall would make the opponent's checkpoint unreachable,
           // this should not be allowed, so we return without setting a wall.

    return;
  }
  }

  /*
  * For each edge in the shortest path from opp_pos to opp_checkpoint,
  *  we check if placing a wall on that edge (in either direction) would increase the opponent's distance to their checkpoint. 
  *  We keep track of the best gain and the corresponding wall placement.
  */
  for (unsigned int i = 0; i + 1 < path_len; i++) {
    vertex_t u = path[i];
    vertex_t v = path[i + 1];
    struct edge_t candidate[2];

    if (find_wall_candidate_for_edge(u, v, candidate)) {
      unsigned int gain = evaluate_wall_gain(candidate, opp_pos,
                                            opp_checkpoint, original_dist);
      if (gain > *gain_ptr) {
        *gain_ptr = gain;
        best_wall[0] = candidate[0];
        best_wall[1] = candidate[1];
      }
    }
    if (find_wall_candidate_for_edge(v, u, candidate)) {
      unsigned int gain = evaluate_wall_gain(candidate, opp_pos,
                                            opp_checkpoint, original_dist);
      if (gain > *gain_ptr) {
        *gain_ptr = gain;
        best_wall[0] = candidate[0];
        best_wall[1] = candidate[1];
      }
    }
  }
}
void finalize() { return; }

