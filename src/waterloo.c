#include <stddef.h>
#include <gsl/gsl_spmatrix.h>

#include "move_check.h"

// Check if wall
int is_wall(struct graph_t graph, vertex_t to) {
  return gsl_spmatrix_uint_get(graph->t, u, v) & WALL_MASK != 0;
}

int move_is_waterloo(struct graph_t* graph,
                     struct move_t move,
                     vertex_t p_pos[NUM_PLAYERS]) {
  vertex_t pos = p_pos[move->color];

  switch (move->t) {
    case MOVE:
      vertex_t v = fr;
      int i = 0;
      while (i < max_dist && v != to) {
        // Find neighbour in dir_go direction from v
        // General implementation for every graph type
        vertex_t u = 0;
        while (u < graph->num_vertices &&
               gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
          u++;
        if (u >= graph->num_vertices ||
            gsl_spmatrix_uint_get(graph->t, v, u) != dir_go)
          return 0;

        // If opponent in path
        if (u == op)
          return 0;

        v = u;
        i++;
      }
    case JUMP:
    case WALL:
    default:
      return 0;
}

