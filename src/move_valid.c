#include <stddef.h>
#include <gsl/gsl_spmatrix.h>
#include <limits.h>
#include <math.h>

#include "move_check.h"

#define MAX_DIST 3
#define MAX_DIST_60_DEG 2

/** Check if 2 vertex are adjacent */
int is_adjacent(struct graph_t* graph, vertex_t u, vertex_t v) {
  return gsl_spmatrix_uint_get(graph->t, u, v) != NO_EDGE;
}

/** Checks if v is a valid vertex */
int is_vertex_valid(struct graph_t* graph, vertex_t v) {
  return v < graph->num_vertices;
}

// --- Validate MOVE type ---

/** Find dir from differences in coordinates of 2 points */
enum dir_t translate_dir(int di, int dj) {
  if (di < 0 && dj < 0) return NW;
  else if (di < 0 && dj == 0) return NE;
  else if (di == 0 && dj > 0) return E;
  else if (di > 0 && dj > 0) return SE;
  else if (di > 0 && dj == 0) return SW;
  else if (di == 0 && dj < 0) return W;
  else return NO_EDGE;
}

/** Difference in path direction. 0 -> straight, 1 -> 60°, else more */
int path_dir_diff(struct graph_t* graph,
                  vertex_t prev,
                  vertex_t fr,
                  vertex_t to,
                  enum dir_t* dir_go) {
  unsigned int n = (unsigned int)sqrt((double)graph->num_vertices);
  int prev_i = prev / n, prev_j = prev % n;
  int fr_i = fr / n, fr_j = fr % n;
  int to_i = to / n, to_j = to % n;

  int di1 = fr_i - prev_i;
  int dj1 = fr_j - prev_j;
  int di2 = to_i - fr_i;
  int dj2 = to_j - fr_j;

  enum dir_t dir1 = translate_dir(di1, dj1);
  enum dir_t dir2 = translate_dir(di2, dj2);
  *dir_go = dir2;

  if (prev == UINT_MAX)  // No previous move
    return NUM_DIRS;

  // Circular directions
  if (dir1 == dir2)
    return 0;
  int diff = abs((int)dir1 - (int)dir2);
  return diff > NUM_DIRS / 2 ? NUM_DIRS - diff : diff;
}

/** Check if exists straight, clear of opponents, path between fr and to of max
 * distance max_dist in direction dir */
int is_straigh_clear_path(struct graph_t* graph,
                          vertex_t to,
                          vertex_t fr,
                          vertex_t op,
                          enum dir_t dir_go,
                          int max_dist) {
  vertex_t v = fr;
  int i = 0;
  while (i < max_dist && v != to) {
    // Find neighbour in dir_go direction from v
    // General implementation for every graph type
    vertex_t u = 0;
    while (u < graph->num_vertices &&
           gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
      u++;
    if (u == graph->num_vertices ||
        gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
      return 0;

    // If opponent in path
    if (u == op)
      return 0;

    v = u;
    i++;
  }

  // to must be found in max_dist loops
  return v == to;
}

/* Validates MOVE type
 * ARGS:
 * - struct graph_t* graph: game graph
 * - vertex_t to:           end move position m
 * - vertex_t fr:           current player position
 * - vertex_t op:           current opponent position
 * - vertex_t prev:         previous player postion
 */
int move_valid(struct graph_t* graph,
               vertex_t to,
               vertex_t fr,
               vertex_t op,
               vertex_t prev) {
  if (!is_vertex_valid(graph, to))
    return 0;
  if (to == fr || to == op)
    return 0;

  enum dir_t dir_go;
  int dir_diff = path_dir_diff(graph, prev, fr, to, &dir_go);

  // straight path
  if (dir_diff == 0)
    return is_straigh_clear_path(graph, to, fr, op, dir_go, MAX_DIST);

  // 60 deg path
  else if (dir_diff == 1)
    return is_straigh_clear_path(graph, to, fr, op, dir_go, MAX_DIST_60_DEG);

  // Angluar path
  else
    return is_adjacent(graph, fr, to);
}

// --- Validate JUMP type ---

/* Validates JUMP type
 * ARGS:
 * - struct graph_t* graph: game graph
 * - vertex_t to:           end move position m
 * - vertex_t fr:           current player position
 * - vertex_t op:           current opponent position
 */
int jump_valid(struct graph_t* graph, vertex_t to, vertex_t fr, vertex_t op) {
  if (!is_vertex_valid(graph, to))
    return 0;
  if (to == fr || to == op)
    return 0;

  return is_adjacent(graph, fr, op) && is_adjacent(graph, op, to);
}

// --- Validate WALL type

/* Validates WALL type
 * ARGS:
 * - struct graph_t* graph: game graph
 * - vertex_t to:           end move position m
 * - vertex_t fr:           current player position
 * - vertex_t op:           current opponent position
 */
int wall_valid(struct graph_t* graph, struct edge_t e[2], int nb_walls) {
  if (nb_walls <= 0)
    return 0;
  if (!is_vertex_valid(graph, e[0].fr) || !is_vertex_valid(graph, e[0].to))
    return 0;
  if (!is_vertex_valid(graph, e[1].fr) || !is_vertex_valid(graph, e[1].to))
    return 0;

  return e[0].fr == e[1].fr && e[0].to != e[1].to &&
         is_adjacent(graph, e[0].fr, e[0].to) &&
         is_adjacent(graph, e[1].fr, e[1].to) &&
         is_adjacent(graph, e[0].to, e[1].to);
}

// --- Main function ---

int move_is_valid(struct graph_t* graph,
                  struct move_t* move,
                  vertex_t p_pos[NUM_PLAYERS],
                  vertex_t p_prev_pos[NUM_PLAYERS],
                  int p_walls[NUM_PLAYERS]) {
  if (move->c >= NUM_PLAYERS)
    return 0;

  switch (move->t) {
    case MOVE:
      return move_valid(graph, move->m, p_pos[move->c], p_pos[1 - move->c],
                        p_prev_pos[move->c]);
    case JUMP:
      return jump_valid(graph, move->m, p_pos[move->c], p_pos[1 - move->c]);
    case WALL:
      return wall_valid(graph, move->e, p_walls[move->c]);
    default:
      return 0;
  }
}
