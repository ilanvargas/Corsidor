#include <stddef.h>
#include <gsl/gsl_spmatrix.h>

#include "move_check.h"
#include <string.h>

#include <stdio.h>
/** Verify if all objectives are visited */
int is_visited_objectives(struct graph_t* graph, int* is_visited) {
  unsigned int num_visited = 0;
  for (unsigned int i = 0; i < graph->num_objectives; i++)
    num_visited += is_visited[graph->objectives[i]];

  return (num_visited == graph->num_objectives) ? 1 : 0;
}

/** Verify if opponent can reach objective position
 * Deep search
 */
int is_block_op(struct graph_t* graph, struct move_t* move, vertex_t op) {
  build_wall(graph, move->e);  // to be removed after

  int is_visited[graph->t->size1];
  memset(is_visited, 0, graph->t->size1 * sizeof(int));

  vertex_t pile[graph->t->size1];
  pile[0] = op;
  int top = 1;
  is_visited[op] = 1;
  while (top > 0 && !is_visited_objectives(graph, is_visited)) {
    vertex_t current = pile[top];
    top--;
    printf("top = %d curr = %d \n", top, current);
    for (unsigned int u = 0; u < graph->t->size1; u++) {
      printf("adj s %d %d\n", current, u);
      if (IS_ADJACENT(graph, current, u) && !is_visited[u]) {
        is_visited[u] = 1;
        printf("%d top = \n", top);
        printf("%d u = \n", top);
        pile[top] = u;
        top++;
      }
    }
  }

  // remove walls
  gsl_spmatrix_uint_set(
      graph->t, move->e[0].fr, move->e[0].to,
      gsl_spmatrix_uint_get(graph->t, move->e[0].fr, move->e[0].to) &
          ~WALL_MASK);
  gsl_spmatrix_uint_set(
      graph->t, move->e[0].to, move->e[0].fr,
      gsl_spmatrix_uint_get(graph->t, move->e[0].to, move->e[0].fr) &
          ~WALL_MASK);

  gsl_spmatrix_uint_set(
      graph->t, move->e[1].fr, move->e[1].to,
      gsl_spmatrix_uint_get(graph->t, move->e[1].fr, move->e[1].to) &
          ~WALL_MASK);
  gsl_spmatrix_uint_set(
      graph->t, move->e[1].to, move->e[1].fr,
      gsl_spmatrix_uint_get(graph->t, move->e[1].to, move->e[1].fr) &
          ~WALL_MASK);

  return is_visited_objectives(graph, is_visited);
}

int convert_waterloo(struct graph_t* graph,
                     struct move_t* move,
                     vertex_t p_pos[NUM_PLAYERS]) {
  vertex_t fr = p_pos[move->c];
  vertex_t op = p_pos[1 - move->c];
  vertex_t to = move->m;

  switch (move->t) {
    case MOVE:
      unsigned int n = (unsigned int)sqrt((double)graph->t->size1);
      int fr_i = fr / n, fr_j = fr % n;
      int to_i = to / n, to_j = to % n;
      enum dir_t dir_go = translate_dir(to_i - fr_i, to_j - fr_j);

      vertex_t v = fr;
      int i = 0;
      while (i < MAX_DIST && v != to) {
        // Find neighbour in dir_go direction from v
        // General implementation for every graph type
        vertex_t u = 0;
        while (IS_VERTEX_VALID(graph, u) &&
               gsl_spmatrix_uint_get(graph->t, v, u) != dir_go &&
               !IS_WALL(graph, v, u))
          u++;
        if (!IS_VERTEX_VALID(graph, u))
          return 0;

        // If wall blocking
        if (IS_WALL(graph, v, u)) {
          move->t = WATERLOO;
          move->m = v;
          move->e[0] = (struct edge_t){v, u};
          move->e[1] = INVALID_EDGE;

          return 1;
        }

        v = u;
        i++;
      }

      return 0;

    case JUMP:
      if (IS_WALL(graph, fr, op)) {
        move->t = WATERLOO;
        move->m = fr;
        move->e[0] = (struct edge_t){fr, op};
        move->e[1] = INVALID_EDGE;

        return 1;
      } else if (IS_WALL(graph, op, to)) {
        move->t = WATERLOO;
        move->m = fr;
        move->e[0] = (struct edge_t){op, to};
        move->e[1] = INVALID_EDGE;

        return 1;
      } else
        return 0;

    case WALL:
      if (IS_WALL(graph, move->e[0].fr, move->e[0].to)) {
        move->t = WATERLOO;
        move->e[1] = INVALID_EDGE;

        return 1;
      } else if (IS_WALL(graph, move->e[1].fr, move->e[1].to)) {
        move->t = WATERLOO;
        move->e[0] = move->e[1];
        move->e[1] = INVALID_EDGE;

        return 1;
      } else if (is_block_op(graph, move, op)) {
        move->t = WATERLOO;
        move->m = INVALID_VERTEX;
        move->e[0] = INVALID_EDGE;
        move->e[1] = INVALID_EDGE;

        return 1;
      } else
        return 0;

    default:
      return 0;
  }
}
