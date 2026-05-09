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
          parent[v] = curr;
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

        int diff = (parent_dir[curr] != NO_EDGE)
                       ? abs((int)parent_dir[curr] - (int)dir_u)
                       : UINT_MAX;
        int dir_diff = (diff > NUM_DIRS / 2) ? NUM_DIRS - diff : diff;

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
  for (unsigned int i = 0; i < num_objectives_to_reach; i++) {
    init_BFS(players_state, source, dist, parent_dir, color);

    BFS(players_state, source, parent, parent_dir, dist, color);

    vertex_t obj =
        next_objective(players_state, i, dist, num_objectives_to_reach,
                       is_objective_reached_path, path->i_objectives);
    printf("next objective %d\n", obj);
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

// find direction between 2 points
enum dir_t find_dir(struct players_state_t* players_state,
                    vertex_t fr,
                    vertex_t to) {
  unsigned int n = (unsigned int)sqrt((double)players_state->graph->t->size1);
  int fr_i = fr / n, fr_j = fr % n;
  int to_i = to / n, to_j = to % n;

  int di = to_i - fr_i;
  int dj = to_j - fr_j;

  if (di < 0 && dj == 0)
    return NW;
  else if (di < 0 && dj < 0)
    return NE;
  else if (di == 0 && dj < 0)
    return E;
  else if (di > 0 && dj == 0)
    return SE;
  else if (di > 0 && dj > 0)
    return SW;
  else if (di == 0 && dj > 0)
    return W;
  else
    return NO_EDGE;
}

void go_throught_path(struct players_state_t* players_state,
                      struct path_t* path,
                      struct move_t* move) {
  vertex_t fr = players_state->pos;
  vertex_t to = move->m;

  enum dir_t dir_go = find_dir(players_state, fr, to);

  vertex_t v = fr;
  int i = 0;
  while (i < MAX_DIST_STRAIGHT && v != to) {
    vertex_t u = 0;
    while (u < players_state->graph->t->size1 &&
           gsl_spmatrix_uint_get(players_state->graph->t, v, u) != dir_go)
      u++;
    if (u >= players_state->graph->t->size1)
      return;  // invalide vertex

    printf("forward %d %d\n", u, dir_go);
    // If opponent in path, we recalculate
    if (u == players_state->op_pos) {
      printf("oponent blocking %d (%d)\n", u, players_state->op_pos);
      // if we can jump
      if (i == 0) {
        for (vertex_t s = 0; s < players_state->graph->t->size1; s++) {
          if (gsl_spmatrix_uint_get(players_state->graph->t, u, s) == dir_go) {
            *move = (struct move_t){.c = players_state->id, .t = JUMP, .m = s};
            break;
          }
        }
      } else {
        *move = (struct move_t){.c = players_state->id, .t = MOVE, .m = v};
      }
      global_glouton(players_state, path);
      return;
    }

    v = u;
    i++;
  }

  // ok to go throught plan
  path->j++;
  if (path->j >= path->lens[path->i]) {
    path->i++;
    path->j = 0;
  }
}
