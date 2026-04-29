#include <stddef.h>
#include <gsl/gsl_spmatrix.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "move_check.h"

#define MAX_DIST 3
#define MAX_DIST_60_DEG 2

/** Checks if m is inside graph */
int is_graph_bound(vertex_t p, struct graph_t* graph) {
  switch (graph->type) {
    case PARALLELOGRAM:
      return p < graph->num_vertices;

    case TRIANGULAR:
      unsigned int n = graph->t->size1;
      unsigned int i = p / n, j = p % n;
      // Find m: seach last connected vertex from first graph line
      unsigned int m = 1;
      while (gsl_spmatrix_uint_get(graph->t, m, m - 1) != NO_EDGE) {
        m++;
      }
      return (j < m + i) && (i < m + j);

    default:
      return p < graph->num_vertices;
  }
}

// --- Validate MOVE type ---

/** Find dir from differences in coordinates of 2 points */
enum dir_t translate_dir(int di, int dj) {
  if (di < 0 && dj < 0)
    return NW;
  else if (di < 0 && dj == 0)
    return NE;
  else if (di == 0 && dj > 0)
    return E;
  else if (di > 0 && dj > 0)
    return SE;
  else if (di > 0 && dj == 0)
    return SW;
  else
    return W;
}

/** Difference in path direction. 0 -> straight, 1 -> 60°, else more */
int path_dir_diff(struct graph_t* graph,
                  vertex_t p0,
                  vertex_t p1,
                  vertex_t p2,
                  enum dir_t* dir_go) {
  if (p0 == UINT_MAX)  // No previous move
    return 6;

  unsigned int n = graph->t->size1;
  int p0i = p0 / n, p0j = p0 % n;
  int p1i = p1 / n, p1j = p1 % n;
  int p2i = p2 / n, p2j = p2 % n;

  int di1 = p1i - p0i;
  int dj1 = p1j - p0j;
  int di2 = p2i - p1i;
  int dj2 = p2j - p1j;

  enum dir_t dir1 = translate_dir(di1, dj1);
  enum dir_t dir2 = translate_dir(di2, dj2);

  *dir_go = dir2;
  return abs((int)dir1 - (int)dir2);
}

/** Check if exists straight, clear of opponents, path between fr and to of max
 * distance max_dist in direction dir */
int is_straigh_clear_path(struct graph_t* graph,
                          vertex_t to,
                          vertex_t fr,
                          vertex_t op_pos,
                          enum dir_t dir_go,
                          int max_dist) {
  vertex_t v = fr;
  int i = 0;
  while (i < MAX_DIST && v != move.m) {
    // Find neighbour in dir_go direction from v
    // General implementation for every graph type
    vertex_t u = 0;
    while (u < graph->num_vertices &&
           gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
      u++;
    if (gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
      return 0;

    // If opponent in path
    if (u == op_pos)
      return 0;

    v = u;
    i++;
  }

  // to must be found in MAX_DIST loops
  return v == to;
}

/* Validates MOVE type
 * ARGS:
 * - struct graph_t* graph: game graph
 * - vertex_t to:           end move position m
 * - vertex_t fr:           current player position
 * - vertex_t op:       current opponent position
 * - vertex_t prev:     previous player postion
 */
int move_valid(struct graph_t* graph,
               vertex_t to,
               vertex_t fr,
               vertex_t op_pos,
               vertex_t prev) {
  if (!is_graph_bound(to, graph))
    return 0;
  if (to == fr || to == op)
    return 0;

  enum dir_t dir_go;
  int dir_diff = path_dir_diff(graph, to, fr, prev, &dir_go);

  // straight path
  if (dir_diff == 0)
    return is_straigh_clear_path(graph, to, fr, op, dir_go, MAX_DIST);

  // 60 deg path
  else if (dir_diff == 1)
    return is_straigh_clear_path(graph, to, fr, op_pos, dir_go,
                                 MAX_DIST_60_DEG);

  // Angluar path
  else
    return gsl_spmatrix_uint_get(graph->t, pos, move.m) != NO_EDGE;
}

// --- Main function ---

int move_is_valid(struct graph_t* graph,
                  struct move_t* move,
                  vertex_t positions[2],
                  vertex_t prev_positions[2]) {
  if (move.c >= NUM_PLAYERS)
    return 0;

  switch (move.t) {
    case MOVE:
      return move_valid(graph, move->m, positions[move.c],
                        positions[1 - move.c], prev_positions[move.c]);
    default:
      return 0;
  }
}
