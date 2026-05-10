#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "move_check.h"
#include "player.h"

char const* p_name = "Foenem";
unsigned int p_id;
struct graph_t* p_graph;
vertex_t pos[NUM_PLAYERS];
vertex_t prev_pos[NUM_PLAYERS];
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
  pos[p_id] = p_graph->start[p_id];
  prev_pos[p_id] = INVALID_VERTEX;
  pos[1 - p_id] = p_graph->start[1 - p_id];
  prev_pos[1 - p_id] = INVALID_VERTEX;
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

vertex_t* valid_adjacent_vertices(struct graph_t* graph,
                                  vertex_t pos,
                                  vertex_t op_pos) {
  unsigned int n = graph->t->size1;
  vertex_t* valid_vertices = malloc(
      NUM_DIRS * sizeof(vertex_t));  // Max 5 adjacent vertices in a hex grid
  unsigned int i = pos / n;
  unsigned int j = pos % n;
  for (unsigned int d = 0; d < NUM_DIRS; d++) {
    valid_vertices[d] = INVALID_VERTEX;  // Initialize as invalid
  }
  vertex_t directions[NUM_DIRS] = {(i - 1) * n + (j - 1), (i - 1) * n + j,
                                   i * n + (j + 1),       (i + 1) * n + (j + 1),
                                   (i + 1) * n + j,       i * n + (j - 1)};
  for (unsigned int d = 0; d < NUM_DIRS; d++) {
    vertex_t next_pos = (vertex_t)directions[d];
    if (IS_VERTEX_VALID(graph, next_pos) && !IS_WALL(graph, pos, next_pos) &&
        next_pos != op_pos) {
      valid_vertices[d] = next_pos;
    }
  }
  return valid_vertices;
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
                        vertex_t prev_pos,
                        unsigned int* dist,
                        vertex_t op_pos,
                        vertex_t* prev) {
  unsigned int n = graph->t->size1;

  for (unsigned int i = 0; i < n; i++) {
    dist[i] = UINT_MAX;
    if (i != source) {
      prev[i] = UINT_MAX;
    }
  }
  prev[source] = prev_pos;
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
    if (prev[u] ==
        (vertex_t)INVALID_VERTEX) {  // If we are at the starting position, we
                                     // can consider all adjacent vertices as
                                     // reachable with distance 1
      vertex_t* valid_vertices = valid_adjacent_vertices(graph, u, op_pos);
      for (unsigned int d = 0; d < NUM_DIRS; d++) {
        vertex_t v = valid_vertices[d];
        if (v != (vertex_t)INVALID_VERTEX) {
          dist[v] = 1;
          prev[v] = u;
        }
      }
      free(valid_vertices);
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
                       vertex_t prev_pos,
                       vertex_t op_pos) {
  unsigned int n = graph->t->size1;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, source, prev_pos, dist, op_pos, prev);
  free(prev);
  return dist;
}

/*
 * Get the closest objective to the current position, and its distance.
 * returns the distance, and sets *next_objective to the closest objective.
 * Only considers objectives that have not been reached yet.
 */
static unsigned int get_closest_objective(struct graph_t* graph,
                                          vertex_t pos,
                                          vertex_t prev_pos,
                                          vertex_t op_pos,
                                          vertex_t* next_objective) {
  unsigned int* dist = djikstra(graph, pos, prev_pos, op_pos);
  unsigned int best_dist = UINT_MAX;
  vertex_t best_objective = UINT_MAX;

  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    if (objective_reached[p_id][i]) {
      continue;  // Skip already reached objectives
    }
    vertex_t objective = graph->objectives[i];
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

static unsigned int get_next_objective(struct graph_t* graph,
                                       vertex_t pos,
                                       vertex_t prev_pos,
                                       vertex_t op_pos,
                                       int player_id) {
  if (objective_left[player_id] == 0) {
    return graph->start[player_id];  // All objectives reached, return
                                     // starting position as next objective
  }
  vertex_t objective = UINT_MAX;
  get_closest_objective(graph, pos, prev_pos, op_pos, &objective);
  return objective;
}
/* Build the shortest path from source to target
 * Returns 1 if a path exists, 0 otherwise. If a path exists, it is stored in
 * the path array and its length is stored in path_len.
 */
static int build_shortest_path(struct graph_t* graph,
                               vertex_t source,
                               vertex_t prev_pos,
                               vertex_t opp_pos,
                               vertex_t target,
                               vertex_t* path,
                               unsigned int* path_len) {
  unsigned int n = graph->num_vertices;
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* prev = malloc(n * sizeof(vertex_t));

  djikstra_with_prev(graph, source, prev_pos, dist, opp_pos, prev);
  if (target == UINT_MAX ||
      dist[target] == UINT_MAX) {  // No path exists or invalid target
    free(dist);
    free(prev);
    *path_len = 0;
    return 0;
  }
  if (dist[target] ==
      UINT_MAX) {  // No path exists (should not happen in a valid game state)
    free(dist);
    free(prev);
    *path_len = 0;
    return 0;
  }

  unsigned int len = 0;
  vertex_t cur = target;
  while (cur != source) {  // Trace back from target to source using prev array
    if (cur == UINT_MAX || len >= n) {  // Avoid infinite loop
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

  /* skip the first movements if we can do an acceleration */
  unsigned int next_index = 0;
  for (unsigned int i = 0; i < len; i++) {
    if (dist[path[i]] == 1) {
      next_index = i;
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

/* Set or clear the wall bit for a given edge in the graph
 * if enable is 1, the wall bit is set, if enable is 0, the wall bit is cleared.
 */
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
unsigned int evaluate_wall_gain(struct graph_t* graph,
                                struct edge_t wall[2],
                                vertex_t pos,
                                vertex_t opp_pos,
                                vertex_t opp_prev_pos,
                                vertex_t opp_checkpoint,
                                unsigned int original_dist) {
  if (!is_valid_edge(graph, wall[0].fr, wall[0].to) ||
      !is_valid_edge(graph, wall[0].to, wall[0].fr) ||
      !is_valid_edge(graph, wall[1].fr, wall[1].to) ||
      !is_valid_edge(graph, wall[1].to, wall[1].fr)) {
    return 0;
  }

  set_wall_mask(graph, wall[0].fr, wall[0].to, 1);
  set_wall_mask(graph, wall[0].to, wall[0].fr, 1);
  set_wall_mask(graph, wall[1].fr, wall[1].to, 1);
  set_wall_mask(graph, wall[1].to, wall[1].fr, 1);

  unsigned int* dist = djikstra(graph, opp_pos, opp_prev_pos, pos);
  unsigned int new_dist = dist[opp_checkpoint];
  free(dist);

  set_wall_mask(graph, wall[0].fr, wall[0].to, 0);
  set_wall_mask(graph, wall[0].to, wall[0].fr, 0);
  set_wall_mask(graph, wall[1].fr, wall[1].to, 0);
  set_wall_mask(graph, wall[1].to, wall[1].fr, 0);

  if (new_dist == UINT_MAX) {
    return 0;  // Opponent cannot reach checkpoint, but we consider the gain as
               // 0 to avoid placing walls that would completely block the
               // opponent
  }
  return (new_dist > original_dist) ? new_dist - original_dist : 0;
}

/*
 * Find candidate walls to place for blocking the adjacent edges to source or
 * target Returns the number of valid candidate wall placements found, and fills
 * the candidate array with the corresponding edges for each valid placement.
 */
int find_walls_candidate_for_edge(struct graph_t* graph,
                                  vertex_t source,
                                  vertex_t op_pos,
                                  vertex_t target,
                                  struct edge_t candidate[2 * NUM_DIRS][2]) {
  vertex_t* adj_src = valid_adjacent_vertices(graph, source, op_pos);
  vertex_t* adj_target = valid_adjacent_vertices(graph, target, op_pos);
  int found_candidate = 0;
  for (unsigned int d1 = 0; d1 < NUM_DIRS; d1++) {
    vertex_t adj_source = adj_src[d1];
    if (adj_source == (vertex_t)INVALID_VERTEX) {
      continue;
    }
    if (d1 == NUM_DIRS - 1) {
      if (adj_src[0] != (vertex_t)INVALID_VERTEX) {
        struct edge_t e1 = {source, adj_source};
        struct edge_t e2 = {source, adj_src[0]};
        candidate[d1][0] = e1;
        candidate[d1][1] = e2;
        found_candidate++;
      }
      break;
    }
    if (adj_src[d1 + 1] != (vertex_t)INVALID_VERTEX) {
      struct edge_t e1 = {source, adj_source};
      struct edge_t e2 = {source, adj_src[d1 + 1]};
      candidate[d1][0] = e1;
      candidate[d1][1] = e2;
      found_candidate++;
    }
  }
  for (unsigned int d2 = 0; d2 < NUM_DIRS; d2++) {
    vertex_t adj_tgt = adj_target[d2];
    if (adj_tgt == (vertex_t)INVALID_VERTEX) {
      continue;
    }
    if (d2 == NUM_DIRS - 1) {
      if (adj_target[0] != (vertex_t)INVALID_VERTEX) {
        struct edge_t e1 = {target, adj_tgt};
        struct edge_t e2 = {target, adj_target[0]};
        candidate[NUM_DIRS + d2][0] = e1;
        candidate[NUM_DIRS + d2][1] = e2;
        found_candidate++;
      }
      break;
    }
    if (adj_target[d2 + 1] != (vertex_t)INVALID_VERTEX) {
      struct edge_t e1 = {target, adj_tgt};
      struct edge_t e2 = {target, adj_target[d2 + 1]};
      candidate[NUM_DIRS + d2][0] = e1;
      candidate[NUM_DIRS + d2][1] = e2;
      found_candidate++;
    }
  }
  free(adj_src);
  free(adj_target);
  return found_candidate;
}

/*
 * Find the best wall to place to increase the opponent's distance to their
 * closest objective. best_wall will contain the edges to place the wall on, and
 * gain_ptr will contain the gain in distance for the opponent.
 */
void find_best_wall(struct graph_t* graph,
                    vertex_t pos,
                    vertex_t opp_pos,
                    vertex_t opp_prev_pos,
                    vertex_t opp_checkpoint,
                    struct edge_t best_wall[2],
                    unsigned int* gain_ptr) {
  unsigned int n = graph->t->size1;
  vertex_t path[n];
  unsigned int path_len = 0;
  unsigned int* opp_dist = djikstra(graph, opp_pos, opp_prev_pos, pos);
  unsigned int original_dist = opp_dist[opp_checkpoint];
  free(opp_dist);

  struct edge_t candidate[2 * NUM_DIRS][2];
  *gain_ptr = 0;
  best_wall[0].fr = UINT_MAX;
  best_wall[0].to = UINT_MAX;
  best_wall[1].fr = UINT_MAX;
  best_wall[1].to = UINT_MAX;

  int path_exists = build_shortest_path(graph, opp_pos, opp_prev_pos, pos,
                                        opp_checkpoint, path, &path_len);
  if (!path_exists) {  // No path exists, no wall can be placed to increase
                       // distance
    return;
  }
  if (path_len == 0) {  // Opp is already at the checkpoint,
    return;
  }
  /*
   * We will try placing walls on the edges adjacent to the opponent's position
   * and the checkpoint, and evaluate the gain for each candidate wall
   * placement.
   */
  int found_candidates = find_walls_candidate_for_edge(
      graph, opp_pos, pos, opp_checkpoint, candidate);
  for (int i = 0; i < found_candidates; i++) {
    if (candidate[i][0].fr == UINT_MAX || candidate[i][0].to == UINT_MAX ||
        candidate[i][1].fr == UINT_MAX || candidate[i][1].to == UINT_MAX) {
      continue;  // Skip invalid candidates
    }
    unsigned int gain =
        evaluate_wall_gain(graph, candidate[i], pos, opp_pos, opp_prev_pos,
                           opp_checkpoint, original_dist);
    if (gain > *gain_ptr) {
      *gain_ptr = gain;
      best_wall[0] = candidate[i][0];
      best_wall[1] = candidate[i][1];
    }
  }
}

static vertex_t get_next_move_vertex(struct graph_t* graph,
                                     vertex_t current_pos,
                                     vertex_t prev_pos,
                                     vertex_t opp_pos,
                                     vertex_t target_pos) {
  if (current_pos == target_pos) {
    return UINT_MAX;  // Already at target, no move along a shortest-path
                      // required
  }

  vertex_t* path = malloc(graph->num_vertices * sizeof(vertex_t));
  unsigned int path_len = 0;

  if (!build_shortest_path(graph, current_pos, prev_pos, opp_pos, target_pos,
                           path,
                           &path_len)) {  // No path exists, should not happen!!
    free(path);
    return UINT_MAX;
  }
  build_shortest_path(graph, current_pos, prev_pos, opp_pos, target_pos, path,
                      &path_len);
  vertex_t next_move = path[0];
  free(path);
  return next_move;
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  if (prev_pos[p_id] == (vertex_t)INVALID_VERTEX) {
    prev_pos[p_id] =
        pos[p_id];  // Initialize previous position on the first move
    if (move_valid(p_graph, pos[p_id], pos[p_id] + 1, pos[1 - p_id],
                   prev_pos[p_id])) {
      pos[p_id] = pos[p_id] + 1;
      return (struct move_t){.c = p_id, .t = MOVE, .m = pos[p_id]};
    } else if (move_valid(p_graph, pos[p_id], pos[p_id] - 1, pos[1 - p_id],
                          prev_pos[p_id])) {
      pos[p_id] = pos[p_id] - 1;
      return (struct move_t){.c = p_id, .t = MOVE, .m = pos[p_id]};
    }
    return (struct move_t){
        .c = p_id,
        .t = NO_TYPE};  // Initialize previous position on the first move
  }

  if (previous_moves[1 - p_id].t == MOVE ||
      previous_moves[1 - p_id].t == JUMP) {
    prev_pos[1 - p_id] = pos[1 - p_id];
    pos[1 - p_id] = previous_moves[1 - p_id].m;
    for (unsigned int k = 0; k < p_graph->num_objectives; k++) {
      if (p_graph->objectives[k] == pos[1 - p_id] &&
          !objective_reached[1 - p_id][k]) {
        objective_reached[1 - p_id][k] = 1;
        objective_left[1 - p_id]--;
        break;
      }
    }
  }
  if (previous_moves[p_id].t ==
      WATERLOO) {  // If we made a waterloo move in the previous turn,
                   // it means a wall was placed on our way,
                   // we need to update our position to the position we ended up
                   // in after the waterloo move, we need to update our graph to
                   // add the wall,
    pos[p_id] = previous_moves[p_id].m;
    // Add the wall that blocked our move
    set_wall_mask(p_graph, previous_moves[p_id].e[0].fr,
                  previous_moves[p_id].e[0].to, 1);
    set_wall_mask(p_graph, previous_moves[p_id].e[0].to,
                  previous_moves[p_id].e[0].fr, 1);
  }

  vertex_t my_checkpoint = get_next_objective(
      p_graph, pos[p_id], prev_pos[p_id], pos[1 - p_id], p_id);
  vertex_t opp_checkpoint = get_next_objective(
      p_graph, pos[1 - p_id], prev_pos[1 - p_id], pos[p_id], 1 - p_id);

  unsigned int* dist_me =
      djikstra(p_graph, pos[p_id], prev_pos[p_id], pos[1 - p_id]);
  unsigned int* dist_opp =
      djikstra(p_graph, pos[1 - p_id], prev_pos[1 - p_id], pos[p_id]);

  // unsigned int d_me = dist_me[my_checkpoint];
  unsigned int d_opp = dist_opp[opp_checkpoint];
  free(dist_me);
  free(dist_opp);

  struct edge_t best_wall[2];
  unsigned int wall_gain = 0;
  find_best_wall(p_graph, pos[p_id], pos[1 - p_id], prev_pos[1 - p_id],
                 opp_checkpoint, best_wall, &wall_gain);

  int should_place_wall = 0;
  if (p_walls_left > 0 && best_wall[0].fr != UINT_MAX) {
    if (wall_gain >= 2) {
      should_place_wall = 1;
    } else if (d_opp <= 2) {
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

  vertex_t next_move = get_next_move_vertex(p_graph, pos[p_id], prev_pos[p_id],
                                            pos[1 - p_id], my_checkpoint);
  /*if (next_move == UINT_MAX) {  // No path to checkpoint or already at target
    next_move = get_any_valid_adjacent_move(p_graph, pos[p_id], prev_pos[p_id],
                                            pos[1 - p_id]);
    if (next_move != UINT_MAX) {
      prev_pos[p_id] = pos[p_id];
      pos[p_id] = next_move;
      return (struct move_t){.c = p_id, .t = MOVE, .m = pos[p_id]};
    }
  }
    */

  /* Check if we reached an objective */
  for (unsigned int i = 0; i < p_graph->num_objectives; i++) {
    if (!objective_reached[p_id][i] && next_move == p_graph->objectives[i]) {
      objective_reached[p_id][i] = 1;
      objective_left[p_id]--;
    }
  }

  prev_pos[p_id] = pos[p_id];
  pos[p_id] = next_move;
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
