#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "graph_impl.h"
#include "move_check.h"
#include "player.h"



char const* p_name = "Foenem";
unsigned int p_id;
struct graph_t* p_graph;
vertex_t p_pos;
vertex_t p_prev_pos;
vertex_t op_pos;
int p_walls_left;
int objective_left[NUM_PLAYERS];
int* objective_reached[NUM_PLAYERS]; /* Track which objectives have been reached
                                      */
char const* get_player_name() {
  return p_name;
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;
  p_graph = graph;
  p_pos = p_graph->start[p_id];
  p_prev_pos = INVALID_VERTEX;
  op_pos = p_graph->start[1 - p_id];
  p_walls_left = graph->num_edges / 16;
  objective_left[p_id] = graph->num_objectives;
  objective_left[1 - p_id] = graph->num_objectives;

  objective_reached[p_id] = malloc(graph->num_objectives * sizeof(int));
  objective_reached[1 - p_id] = malloc(graph->num_objectives * sizeof(int));
  /* Initialize objective tracking */
  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    objective_reached[p_id][i] = 0;
    objective_reached[1 - p_id][i] = 0;
  }
}

/*
 * Check if a target is reachable according to move validation rules.
 * Returns 1 if reachable, 0 otherwise.
 */
int is_move_reachable(struct graph_t* graph,
                      vertex_t pos,
                      vertex_t next_step,
                      vertex_t prev_pos,
                      vertex_t op_pos) {
  return move_valid(graph, next_step, pos, op_pos, prev_pos);
}

void djikstra_with_prev(struct graph_t* graph,
                        vertex_t source,
                        unsigned int* dist,
                        vertex_t op_pos,
                        vertex_t* prev) {
  unsigned int n = graph->t->size1;

  for (unsigned int i = 0; i < n; i++) {
    dist[i] = UINT_MAX;
    if (i != source) {
      prev[i] = UINT_MAX;
    }
    prev[source] = source;
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

    if (u == UINT_MAX || best_dist == UINT_MAX) {  // No more reachable vertices
      break;
    }

    visited[u] = 1;
    if (u == graph->start[p_id]) {  // If we are at the starting position, we can consider all adjacent vertices as reachable with distance 1
      for (unsigned int v = 0; v < n; v++) {
        if (IS_ADJACENT(graph, u, v) && !IS_WALL(graph, u, v) && v != op_pos) {
          dist[v] = 1;
          prev[v] = u;
        }
      }
      continue;
    }
    /* Relax edges */
    for (unsigned int v = 0; v < n; v++) {
      if (is_move_reachable(graph, u, v, prev[u], op_pos)) {
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
 * Get the distance from source to all vertices using Djikstra's algorithm.
 * Returns an array of distances, where dist[i] is the distance from source to
 * vertex i. If a vertex is unreachable from source, its distance will be
 * UINT_MAX.
 */
unsigned int* djikstra(struct graph_t* graph,
                       vertex_t source,
                       vertex_t op_pos) {
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, source, dist, op_pos, prev);
  free(prev);
  return dist;
}

/*
 * Get the closest objective to the current position, and its distance.
 * returns the distance, and sets *next_objective to the closest objective.
 * Only considers objectives that have not been reached yet.
 */
static unsigned int get_closest_objective(vertex_t pos,
                                          vertex_t op_pos,
                                          vertex_t* next_objective) {
  unsigned int* dist = djikstra(p_graph, pos, op_pos);
  unsigned int best_dist = UINT_MAX;
  vertex_t best_objective = UINT_MAX;

  for (unsigned int i = 0; i < p_graph->num_objectives; i++) {
    if (objective_reached[p_id][i]) {
      continue; /* Skip already reached objectives */
    }
    vertex_t objective = p_graph->objectives[i];
    unsigned int effective_dist = dist[objective];
    if (effective_dist < best_dist) {
      best_dist = effective_dist;
      best_objective = objective;
    }
  }

  free(dist);
  *next_objective = best_objective;
  return best_dist;
}

static unsigned int get_next_objective(vertex_t pos,
                                       vertex_t op_pos,
                                       int player_id) {
  if (objective_left[player_id] == 0) {
    return p_graph->start[player_id];  // All objectives reached, return
                                       // starting position as next objective
  }
  vertex_t objective = UINT_MAX;
  get_closest_objective(pos, op_pos, &objective);
  if (objective == UINT_MAX) {
    return p_graph->start[player_id];
  }
  return objective;
}
/* Build the shortest path from source to target
 * Returns 1 if a path exists, 0 otherwise. If a path exists, it is stored in
 * the path array and its length is stored in path_len.
 */
static int build_shortest_path(vertex_t source,
                               vertex_t target,
                               vertex_t* path,
                               unsigned int* path_len) {
  unsigned int n = p_graph->num_vertices;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(p_graph, source, dist, op_pos, prev);

  if (dist[target] ==
      UINT_MAX) {  // No path exists (should not happen in a valid game state)
    free(dist);
    free(prev);
    *path_len = 0;
    return 0;
  }

  unsigned int len = 0;
  vertex_t cur = target;
  while (cur !=
         source) {  // Trace back from target to source using prev array
    if (cur == UINT_MAX || len >= n) {
      free(dist);
      free(prev);
      *path_len = 0;
      return 0;
    }
    path[len] = cur;
    len++;
    cur = prev[cur];
  }

  for (unsigned int i = 0; i < len / 2;
       i++) {  // Reverse the path to get it from source to target
    vertex_t tmp = path[i];
    path[i] = path[len - 1 - i];
    path[len - 1 - i] = tmp;
  }

  unsigned int next_index = 0;
  for (unsigned int i = 0; i < len; i++) {
    if (dist[path[i]] == 1) {
      next_index = i;
      break;
    }
  }
  for (unsigned int i = 0; i < len - next_index; i++) {
    path[i] = path[i + next_index];
  }
  len -= next_index;
  free(dist);
  free(prev);

  *path_len = len;
  return 1;
}

static int is_valid_edge(struct graph_t* graph, vertex_t fr, vertex_t to) {
  return fr < graph->t->size1 && to < graph->t->size1;
}

/* Set or clear the wall bit for a given edge in the graph */
static void set_wall_mask(struct graph_t* graph,
                          vertex_t fr,
                          vertex_t to,
                          int enable) {
  if (!is_valid_edge(graph, fr, to)) {
    return;
  }
  unsigned int value = gsl_spmatrix_uint_get(graph->t, fr, to);
  if (enable) {
    value |= WALL_MASK;  // Set the wall bit
  } else {
    value &= ~WALL_MASK;  // Clear the wall bit
  }
  gsl_spmatrix_uint_set(graph->t, fr, to, value);
}

/* Evaluate the gain from placing walls at the specified edges
 * Returns the increase in distance for the opponent to reach their checkpoint,
 * or 0 if there is no increase, or UINT_MAX if it becomes impossible for the
 * opponent to reach their checkpoint.
 */
static unsigned int evaluate_wall_gain(struct edge_t wall[2],
                                       vertex_t pos,
                                       vertex_t opp_pos,
                                       vertex_t opp_checkpoint,
                                       unsigned int original_dist) {
  if (!is_valid_edge(p_graph, wall[0].fr, wall[0].to) ||
      !is_valid_edge(p_graph, wall[0].to, wall[0].fr) ||
      !is_valid_edge(p_graph, wall[1].fr, wall[1].to) ||
      !is_valid_edge(p_graph, wall[1].to, wall[1].fr)) {
    return 0;
  }

  set_wall_mask(p_graph, wall[0].fr, wall[0].to, 1);
  set_wall_mask(p_graph, wall[0].to, wall[0].fr, 1);
  set_wall_mask(p_graph, wall[1].fr, wall[1].to, 1);
  set_wall_mask(p_graph, wall[1].to, wall[1].fr, 1);

  unsigned int* dist = djikstra(p_graph, opp_pos, pos);
  unsigned int new_dist = dist[opp_checkpoint];
  free(dist);

  set_wall_mask(p_graph, wall[0].fr, wall[0].to, 0);
  set_wall_mask(p_graph, wall[0].to, wall[0].fr, 0);
  set_wall_mask(p_graph, wall[1].fr, wall[1].to, 0);
  set_wall_mask(p_graph, wall[1].to, wall[1].fr, 0);

  if (original_dist == UINT_MAX || new_dist == UINT_MAX) {
    return (new_dist == UINT_MAX && original_dist != UINT_MAX) ? UINT_MAX : 0;
  }
  return (new_dist > original_dist) ? new_dist - original_dist : 0;
}

/*
 * Find a pair of edges that can be used to place a wall between source and
 * target Returns 1 if such a pair of edges is found, 0 otherwise. If found, the
 * candidate edges are stored in the candidate array.
 */
static int find_wall_candidate_for_edge(vertex_t source,
                                        vertex_t target,
                                        struct edge_t candidate[2]) {
  unsigned int n = p_graph->t->size1;

  for (unsigned int other = 0; other < n; other++) {
    if (other == target) {
      continue;
    }
    if (!IS_ADJACENT(p_graph, source, other) ||
        !IS_ADJACENT(
            p_graph, target,
            other)) {  // other must be adjacent to both source and target
      continue;
    }

    if (IS_WALL(p_graph, source, other) || IS_WALL(p_graph, source, target)) {
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
 * Find the best wall to place to increase the opponent's distance to their
 * closest objective. best_wall will contain the edges to place the wall on, and
 * gain_ptr will contain the gain in distance for the opponent.
 */
static void find_best_wall(vertex_t pos,
                           vertex_t opp_pos,
                           vertex_t opp_checkpoint,
                           struct edge_t best_wall[2],
                           unsigned int* gain_ptr) {
  unsigned int n = p_graph->t->size1;
  vertex_t path[n];
  unsigned int path_len = 0;
  unsigned int* opp_dist = djikstra(p_graph, opp_pos, pos);
  unsigned int original_dist = opp_dist[opp_checkpoint];
  free(opp_dist);

  *gain_ptr = 0;
  best_wall[0].fr = UINT_MAX;
  best_wall[0].to = UINT_MAX;
  best_wall[1].fr = UINT_MAX;
  best_wall[1].to = UINT_MAX;

  int path_exists =
      build_shortest_path(opp_pos, opp_checkpoint, path, &path_len);
  if (!path_exists) {  // No path exists, no wall can be placed to increase
                       // distance
    return;
  }
  if (path_len == 0) {  // Opp is already at the checkpoint,
    return;
  }
  if (path_len == 1) {  // Opp and opp_checkpoint are adjacent,
                        // so we consider it as the best wall with maximum gain
    struct edge_t candidate[2];
    if (!find_wall_candidate_for_edge(opp_pos, opp_checkpoint, candidate)) {
      return;
    }

    unsigned int gain = evaluate_wall_gain(candidate, pos, opp_pos,
                                           opp_checkpoint, original_dist);
    if (gain != UINT_MAX) {
      *gain_ptr = gain;
      best_wall[0] = candidate[0];
      best_wall[1] = candidate[1];
    }
    return;
  }
  /*
   * For each edge in the shortest path from opp_pos to opp_checkpoint,
   *  we check if placing a wall on that edge (in either direction) would
   * increase the opponent's distance to their checkpoint. We keep track of the
   * best gain and the corresponding wall placement.
   */
  for (unsigned int i = 0; i + 1 < path_len; i++) {
    vertex_t u = path[i];
    vertex_t v = path[i + 1];
    struct edge_t candidate[2];
    int cond = find_wall_candidate_for_edge(u, v, candidate);
    if (cond) {
      unsigned int gain = evaluate_wall_gain(candidate, pos, opp_pos,
                                             opp_checkpoint, original_dist);
      if (gain > *gain_ptr) {
        *gain_ptr = gain;
        best_wall[0] = candidate[0];
        best_wall[1] = candidate[1];
      }
    }
    if (find_wall_candidate_for_edge(v, u, candidate)) {
      unsigned int gain = evaluate_wall_gain(candidate, pos, opp_pos,
                                             opp_checkpoint, original_dist);
      if (gain > *gain_ptr) {
        *gain_ptr = gain;
        best_wall[0] = candidate[0];
        best_wall[1] = candidate[1];
      }
    }
  }
}

static vertex_t get_next_move_vertex(vertex_t current_pos,
                                     vertex_t target_pos) {
  vertex_t* path = malloc(p_graph->num_vertices * sizeof(vertex_t));
  unsigned int path_len = 0;

  if (!build_shortest_path(current_pos, target_pos, path,
                           &path_len)) {  // No path exists, should not happen!!
    return UINT_MAX;
  }
  build_shortest_path(current_pos, target_pos, path, &path_len);
  vertex_t next_move = path[0];
  free(path);
  return next_move;
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  /*if (p_prev_pos == (vertex_t)INVALID_VERTEX) {
    p_prev_pos = p_pos;  // Initialize previous position on the first move
    if (move_valid(p_graph, p_pos, p_pos+1, op_pos, p_prev_pos)) {
      p_pos = p_pos + 1;
      return (struct move_t){.c = p_id, .t = MOVE, .m = p_pos};
    } else if (move_valid(p_graph, p_pos, p_pos-1, op_pos, p_prev_pos)) {
      p_pos = p_pos - 1;
      return (struct move_t){.c = p_id, .t = MOVE, .m = p_pos};
    }
    return (struct move_t){.c = p_id, .t = NO_TYPE};  // Initialize previous position on the first move
  }
  */
  if (previous_moves[1 - p_id].t == MOVE ||
      previous_moves[1 - p_id].t == JUMP) {
    op_pos = previous_moves[1 - p_id].m;
  }
  if (previous_moves[p_id].t ==
      WATERLOO) {  // If we made a waterloo move in the previous turn,
                   // it means a wall was placed on our way,
                   // we need to update our position to the position we ended up
                   // in after the waterloo move, we need to update our graph to
                   // add the wall,
    p_pos = previous_moves[p_id].m;
    // Add the wall that blocked our move
    set_wall_mask(p_graph, previous_moves[p_id].e[0].fr,
                  previous_moves[p_id].e[0].to, 1);
    set_wall_mask(p_graph, previous_moves[p_id].e[0].to,
                  previous_moves[p_id].e[0].fr, 1);
  }

  vertex_t my_checkpoint = get_next_objective(p_pos, op_pos, p_id);
  vertex_t opp_checkpoint = get_next_objective(op_pos, p_pos, 1 - p_id);

  unsigned int* dist_me = djikstra(p_graph, p_pos, op_pos);
  unsigned int* dist_opp = djikstra(p_graph, op_pos, p_pos);

  // unsigned int d_me = dist_me[my_checkpoint];
  unsigned int d_opp = dist_opp[opp_checkpoint];
  free(dist_me);
  free(dist_opp);

  struct edge_t best_wall[2];
  unsigned int wall_gain = 0;
  find_best_wall(p_pos, op_pos, opp_checkpoint, best_wall, &wall_gain);

  int should_place_wall = 0;
  if (p_walls_left > 0 && best_wall[0].fr != UINT_MAX) {
    if (wall_gain >= 4) {
      should_place_wall = 1;
    } else if (wall_gain >= 2 && d_opp <= 2) {
      should_place_wall = 1;
    }
  }

  if (should_place_wall) {  // Place the wall if it provides a significant gain
                            // or if the opponent is very close to their
                            // checkpoint
    p_walls_left--;
    set_wall_mask(p_graph, best_wall[0].fr, best_wall[0].to, 1);
    set_wall_mask(p_graph, best_wall[0].to, best_wall[0].fr, 1);
    set_wall_mask(p_graph, best_wall[1].fr, best_wall[1].to, 1);
    set_wall_mask(p_graph, best_wall[1].to, best_wall[1].fr, 1);
    return (struct move_t){
        .c = p_id, .t = WALL, .e = {best_wall[0], best_wall[1]}};
  }

  vertex_t next_move = get_next_move_vertex(p_pos, my_checkpoint);
  if (next_move == UINT_MAX) {  // No path to checkpoint, should not happen!!
    return (struct move_t){.c = p_id, .t = MOVE, .m = p_pos};
  }

  /* Check if we reached an objective */
  for (unsigned int i = 0; i < p_graph->num_objectives; i++) {
    if (!objective_reached[p_id][i] && next_move == p_graph->objectives[i]) {
      objective_reached[p_id][i] = 1;
      objective_left[p_id]--;
    }
  }

  p_prev_pos = p_pos;
  p_pos = next_move;
  return (struct move_t){.c = p_id, .t = MOVE, .m = next_move};
}

void finalize() {
  gsl_spmatrix_uint_free(p_graph->t);
  free(p_graph->objectives);
  free(p_graph);

  free(objective_reached[p_id]);
  free(objective_reached[1 - p_id]);
  return;
}
