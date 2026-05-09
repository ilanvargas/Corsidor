#include <string.h>
#include <stdlib.h>

#include "player_tourist.h"

#include <stdio.h>
/* BFS helper
 * Enqueue and calulate dist, parent, parent_dir in vertex further than curr
 * neighbours Usefull for straight and 60 deg case
 */
void queue_deep(struct players_state_t* players_state,
                vertex_t curr,
                vertex_t* parent,
                enum dir_t* parent_dir,
                unsigned int* dist,
                int* color,
                int max_dist,
                enum dir_t dir_u,
                vertex_t* queue,
                int* tail) {
  vertex_t curr_straight = curr;
  for (int i = 0; i < max_dist; i++) {
    for (vertex_t v = 0; v < players_state->graph->t->size1; v++) {
      if (gsl_spmatrix_uint_get(players_state->graph->t, curr_straight, v) ==
          dir_u) {
        if (color[v] == 0) {
          dist[v] = dist[curr] + 1;
          parent[v] = curr_straight;
          parent_dir[v] = parent_dir[curr];

          color[v] = 1;
          queue[*tail] = v;
          (*tail)++;
        }
        curr_straight = v;
        break;
      }
    }
  }
}

/** Calculates the difference of 2 dirs taking into accont circular propreties*/
int dir_difference(enum dir_t prev_dir, enum dir_t next_dir) {
  int diff =
      (prev_dir != NO_EDGE) ? abs((int)prev_dir - (int)next_dir) : UINT_MAX;
  return (diff > NUM_DIRS / 2) ? NUM_DIRS - diff : diff;
}

/** BFS (parcours en largeur) algorithm to find distances from source
 * ARGS:
 * - struct players_state_t* players_state: game state;
 * - vertex_t source: vertex to calculate distaces from;
 * - vertex_t* parent: parent[i] contains predecessor from vertex i;
 * - enum dir_t* parent_dir: parent[i] contains direction followd to go to i;
 * - unsigned int* dist: dist[i] distance to go from source to i;
 * - int* color: color[i] vertex color white, grey or black;
 * dist, parent, parent_dir are modified at the end
 * Since all edges are wheight 1, BFS is enougth to calculate distances
 */
void BFS(struct players_state_t* players_state,
         vertex_t source,
         vertex_t* parent,
         enum dir_t* parent_dir,
         unsigned int* dist,
         int* color) {
  unsigned int n = players_state->graph->t->size1;

  vertex_t queue[players_state->graph->t->size1];
  queue[0] = source;
  int head = 0;
  int tail = 1;

  color[source] = 1;
  dist[source] = 0;

  // while queue not empty
  while (head != tail) {
    vertex_t curr = queue[head++];

    for (vertex_t u = 0; u < n; u++) {
      enum dir_t dir_u =
          gsl_spmatrix_uint_get(players_state->graph->t, curr, u);

      // for every neighbour of curr
      if (dir_u != NO_EDGE) {
        // if never visited
        if (color[u] == 0) {
          dist[u] = dist[curr] + 1;
          parent[u] = curr;
          parent_dir[u] =
              gsl_spmatrix_uint_get(players_state->graph->t, curr, u);

          color[u] = 1;
          queue[tail++] = u;
        }

        int dir_diff = dir_difference(parent_dir[curr], dir_u);

        // can advance up to MAX_DIST_STRAIGHT vertex straight ahead
        if (dir_diff == 0)
          queue_deep(players_state, curr, parent, parent_dir, dist, color,
                     MAX_DIST_STRAIGHT, dir_u, queue, &tail);

        // can advance up to MAX_DIST_60 vertex 60 deg
        if (dir_diff == 1)
          queue_deep(players_state, curr, parent, parent_dir, dist, color,
                     MAX_DIST_60, dir_u, queue, &tail);
      }
    }

    color[curr] = 2;
  }
}

void init_BFS(struct players_state_t* players_state,
              vertex_t source,
              unsigned int* dist,
              enum dir_t* parent_dir,
              int* color) {
  for (unsigned int j = 0; j < players_state->graph->t->size1; j++) {
    dist[j] = UINT_MAX;
  }
  dist[source] = 0;
  parent_dir[source] = NO_EDGE;
  memset(color, 0, players_state->graph->t->size1 * sizeof(int));
}

/** Retuns next objective
 * Nearest objective or starting point if all other objectives done
 * Updates paths_i_objectives and is_objective_reached_path
 */
vertex_t next_objective(struct players_state_t* players_state,
                        unsigned int i,
                        unsigned int* dist,
                        unsigned int num_objectives_to_reach,
                        int* is_objective_reached_path,
                        unsigned int* paths_i_objectives) {
  if (i == num_objectives_to_reach)
    return players_state->graph->start[players_state->id];

  unsigned int min_i_obj = players_state->graph->num_objectives;
  unsigned int dist_min_obj = UINT_MAX;
  for (unsigned int j = 0; j < players_state->graph->num_objectives; j++) {
    if (dist[players_state->graph->objectives[j]] < dist_min_obj &&
        !is_objective_reached_path[j]) {
      min_i_obj = j;
      dist_min_obj = dist[players_state->graph->objectives[j]];
    }
  }

  if (min_i_obj == players_state->graph->num_objectives)
    return players_state->graph->start[players_state->id];

  paths_i_objectives[i] = min_i_obj;
  is_objective_reached_path[min_i_obj] = 1;

  return players_state->graph->objectives[min_i_obj];
}

/** path creation from parent BFS
 * Puts into path[i] arrray
 */
void path_creation(struct players_state_t* players_state,
                   struct path_t* path,
                   vertex_t source,
                   vertex_t obj,
                   unsigned int i,
                   vertex_t* parent) {
  vertex_t curr_path[players_state->graph->t->size1];
  size_t curr_path_len = 0;
  vertex_t v = obj;
  while (v != source) {
    curr_path[curr_path_len] = v;
    v = parent[v];
    curr_path_len++;
  }

  for (size_t j = 0; j < curr_path_len; j++) {
    path->paths[i][j] = curr_path[curr_path_len - j - 1];
  }
  path->lens[i] = curr_path_len;

  printf("path %d: ", i);
  for (size_t j = 0; j < path->lens[i]; j++)
    printf("%d ", path->paths[i][j]);
  printf("\n");
}

void global_glouton(struct players_state_t* players_state,
                    struct path_t* path) {
  unsigned int n = players_state->graph->t->size1;
  unsigned int total_dist = 0;

  // to count all objectives reached in path
  int is_objective_reached_path[players_state->graph->num_objectives];
  unsigned int num_objectives_to_reach = 0;
  for (unsigned int i = 0; i < players_state->graph->num_objectives; i++) {
    is_objective_reached_path[i] =
        players_state->is_objective_reached[players_state->id][i];
    num_objectives_to_reach +=
        !players_state->is_objective_reached[players_state->id][i];
  }

  // dynamic allocation
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* parent = malloc(n * sizeof(vertex_t));
  enum dir_t* parent_dir = malloc(n * sizeof(enum dir_t));
  int* color = malloc(n * sizeof(int));

  vertex_t source = players_state->pos;
  for (unsigned int i = 0; i < num_objectives_to_reach + 1; i++) {
    init_BFS(players_state, source, dist, parent_dir, color);

    BFS(players_state, source, parent, parent_dir, dist, color);

    vertex_t obj =
        next_objective(players_state, i, dist, num_objectives_to_reach,
                       is_objective_reached_path, path->i_objectives);
    path_creation(players_state, path, source, obj, i, parent);

    // preparation for next path
    source = obj;
    path->dists[i] = dist[obj];
    total_dist += dist[obj];
  }

  free(dist);
  free(parent);
  free(parent_dir);
  free(color);

  // initialize path pointers
  path->i = 0;
  path->j = 0;

  path->total_dist = total_dist;
  path->origin_source = source;
}

struct move_t go_throught_path(struct players_state_t* players_state,
                               struct path_t* path) {
  unsigned int n = players_state->graph->t->size1;
  vertex_t fr = players_state->pos;
  vertex_t next = path->paths[path->i][path->j];
  enum dir_t dir_go = gsl_spmatrix_uint_get(players_state->graph->t, fr, next);

  int dir_diff = dir_difference(players_state->prev_dir, dir_go);
  int max_dist;
  if (dir_diff == 0)
    max_dist = MAX_DIST_STRAIGHT;
  else if (dir_diff == 1)
    max_dist = MAX_DIST_60;
  else
    max_dist = 1;

  players_state->prev_dir = dir_go;

  vertex_t v = fr;
  for (int i = 0; i < max_dist; i++) {
    next = path->paths[path->i][path->j];
    enum dir_t next_dir =
        gsl_spmatrix_uint_get(players_state->graph->t, v, next);

    if (next_dir != dir_go)
      break;

    // if opponent blocking path
    if (next == players_state->op_pos) {
      // if we can jump
      if (i == 0) {
        for (vertex_t u = 0; u < n; u++) {
          if (gsl_spmatrix_uint_get(players_state->graph->t, next, u) ==
              dir_go) {
            players_state->pos = u;
            global_glouton(players_state, path);
            return (struct move_t){.c = players_state->id, .t = JUMP, .m = u};
          }
        }
        // if not able to go in dir_go
        for (vertex_t u = 0; u < n; u++) {
          if (gsl_spmatrix_uint_get(players_state->graph->t, next, u) !=
              NO_EDGE) {
            players_state->pos = u;
            global_glouton(players_state, path);
            return (struct move_t){.c = players_state->id, .t = JUMP, .m = u};
          }
        }
      }
      // just stops in from of player
      else {
        players_state->pos = v;
        global_glouton(players_state, path);
        break;
      }
    }

    // move forward path
    v = next;

    path->j++;
    if (path->j >= path->lens[path->i]) {
      path->i++;
      path->j = 0;

      // update objetive_reached
      for (unsigned int k = 0; k < players_state->graph->num_objectives; k++) {
        if (players_state->graph->objectives[k] == v &&
            !players_state->is_objective_reached[players_state->id][k]) {
          players_state->is_objective_reached[players_state->id][k] = 1;
          players_state->num_objective_to_reach--;
          break;
        }
      }
      break;
    }
  }

  players_state->pos = v;
  return (struct move_t){.c = players_state->id, .t = MOVE, .m = v};
}
