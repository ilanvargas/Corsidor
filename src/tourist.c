#include <string.h>
#include <stdlib.h>

#include "player_tourist.h"

struct players_state_t* init_players_state(unsigned int player_id,
                                           struct graph_t* graph) {
  struct players_state_t* players_state =
      malloc(sizeof(struct players_state_t));

  players_state->id = player_id;
  players_state->graph = graph;
  players_state->pos = graph->start[player_id];
  players_state->op_pos = graph->start[1 - player_id];
  players_state->prev_dir = NO_EDGE;
  players_state->prev_move = (struct move_t){.t = NO_TYPE};
  players_state->num_walls = graph->num_edges / 16;

  players_state->is_objective_reached[player_id] =
      malloc(graph->num_objectives * sizeof(int));
  players_state->is_objective_reached[1 - player_id] =
      malloc(graph->num_objectives * sizeof(int));
  memset(players_state->is_objective_reached[player_id], 0,
         graph->num_objectives * sizeof(int));
  memset(players_state->is_objective_reached[1 - player_id], 0,
         graph->num_objectives * sizeof(int));

  players_state->num_objective_to_reach = graph->num_objectives;

  return players_state;
}

void free_players_state(struct players_state_t* players_state) {
  for (int i = 0; i < NUM_PLAYERS; i++)
    free(players_state->is_objective_reached[i]);

  gsl_spmatrix_uint_free(players_state->graph->t);
  free(players_state->graph->objectives);
  free(players_state->graph);

  free(players_state);
}

struct path_t* init_path(struct graph_t* graph) {
  struct path_t* path = malloc(sizeof(struct path_t));

  // graph->num_objectives+1 because pos -> objectives -> starting point
  int num_paths = graph->num_objectives + 1;
  path->paths = malloc(num_paths * sizeof(vertex_t*));
  for (unsigned int i = 0; i < graph->num_objectives + 1; i++) {
    path->paths[i] = malloc((graph->t->size1) * sizeof(vertex_t));  // one path
  }

  path->dists = malloc(num_paths * sizeof(unsigned int));
  path->lens = malloc(num_paths * sizeof(size_t));
  path->i_objectives = malloc(graph->num_objectives * sizeof(unsigned int));

  return path;
}

void free_path(struct path_t* path, struct players_state_t* players_state) {
  for (unsigned int i = 0; i < players_state->graph->num_objectives + 1; i++)
    free(path->paths[i]);
  free(path->paths);

  free(path->dists);
  free(path->lens);
  free(path->i_objectives);

  free(path);
}

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
  gsl_spmatrix_uint* csr = players_state->graph->t;

  vertex_t curr_straight = curr;
  for (int i = 0; i < max_dist; i++) {

    for (int k = csr->p[curr_straight]; k < csr->p[curr_straight + 1]; k++) {
      vertex_t v = csr->i[k];
      if (csr->data[k] == dir_u) {
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
  gsl_spmatrix_uint* csr = players_state->graph->t;

  vertex_t queue[csr->size1];
  queue[0] = source;
  int head = 0;
  int tail = 1;

  color[source] = 1;
  dist[source] = 0;

  // while queue not empty
  while (head != tail) {
    vertex_t curr = queue[head++];

    for (int k = csr->p[curr]; k < csr->p[curr + 1]; k++) {
      vertex_t u = csr->i[k];
      unsigned int dir_u = csr->data[k];

      // for every accessible neighbour of curr
      if (!IS_WALL(dir_u)) {
        // if never visited
        if (color[u] == 0) {
          dist[u] = dist[curr] + 1;
          parent[u] = curr;
          parent_dir[u] = dir_u;

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
  unsigned int n = players_state->graph->t->size1;
  for (unsigned int j = 0; j < n; j++) {
    dist[j] = UINT_MAX;
    parent_dir[j] = NO_EDGE;  
  }
  dist[source] = 0;
  memset(color, 0, n * sizeof(int));
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
  for (size_t j = 0; j < curr_path_len; j++)
    printf(" %d ", path->paths[i][j]);
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

/* Update player objectives */
void update_objectives(struct players_state_t* players_state, vertex_t v) {
  for (unsigned int k = 0; k < players_state->graph->num_objectives; k++) {
    if (players_state->graph->objectives[k] == v &&
        !players_state->is_objective_reached[players_state->id][k]) {
      players_state->is_objective_reached[players_state->id][k] = 1;
      players_state->num_objective_to_reach--;
      break;
    }
  }
}

struct move_t go_throught_path(struct players_state_t* players_state,
                               struct path_t* path) {
  gsl_spmatrix_uint* csr = players_state->graph->t;
  vertex_t fr = players_state->pos;
  vertex_t next = path->paths[path->i][path->j];
  enum dir_t dir_go = gsl_spmatrix_uint_get(csr, fr, next);

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
    enum dir_t next_dir = gsl_spmatrix_uint_get(csr, v, next);

    if (next_dir != dir_go)
      break;

    // if opponent blocking path
    if (next == players_state->op_pos) {
      // if we can jump
      if (i == 0) {
        for (int k = csr->p[next]; k < csr->p[next + 1]; k++) {
          if (csr->data[k] == dir_go) {
            vertex_t u = csr->i[k];

            players_state->pos = u;
            update_objectives(players_state, u);
            global_glouton(players_state, path);
            return (struct move_t){.c = players_state->id, .t = JUMP, .m = u};
          }
        }
        // if not able to go in dir_go
        for (int k = csr->p[next]; k < csr->p[next + 1]; k++) {
          vertex_t u = csr->i[k];

          players_state->pos = u;
          update_objectives(players_state, u);
          global_glouton(players_state, path);
          return (struct move_t){.c = players_state->id, .t = JUMP, .m = u};
        }
      }
      // just stops in from of player
      else {
        players_state->pos = v;
        update_objectives(players_state, v);
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
      update_objectives(players_state, v);

      break;
    }
  }

  players_state->pos = v;
  return (struct move_t){.c = players_state->id, .t = MOVE, .m = v};
}

/** Gives to a pointer to edge
 * Usefull to modify it (add wall)
 */
unsigned int* get_edge(gsl_spmatrix_uint* csr, vertex_t fr, vertex_t to) {
    for (int k = csr->p[fr]; k < csr->p[fr + 1]; k++) {
        if ((vertex_t)csr->i[k] == to)
            return &csr->data[k];
    }
    return NULL;
}

void build_wall(struct players_state_t* players_state, struct edge_t e) {
  // Wall needs to be read on both sides
  gsl_spmatrix_uint* csr = players_state->graph->t;

  unsigned int* val_fr = get_edge(csr, e.fr, e.to);
  unsigned int* val_to = get_edge(csr, e.to, e.fr);

  *val_to |= WALL_MASK;
  *val_fr |= WALL_MASK;
}

void remove_wall(struct players_state_t* players_state, struct edge_t e) {
  gsl_spmatrix_uint* csr = players_state->graph->t;

  unsigned int* val_fr = get_edge(csr, e.fr, e.to);
  unsigned int* val_to = get_edge(csr, e.to, e.fr);

  *val_to &= ~WALL_MASK;
  *val_fr &= ~WALL_MASK;
}

int wall_move(struct players_state_t* players_state,
              struct path_t* path,
              struct move_t* move) {
  if (players_state->num_walls <= 0)
    return 0;

  gsl_spmatrix_uint* csr = players_state->graph->t;
  unsigned int op_id = 1 - players_state->id;
  unsigned int n = csr->size1;

  // dynamic allocation
  unsigned int* dist = malloc(n * sizeof(unsigned int));
  vertex_t* parent = malloc(n * sizeof(vertex_t));
  enum dir_t* parent_dir = malloc(n * sizeof(enum dir_t));
  int* color = malloc(n * sizeof(int));

  vertex_t source = players_state->op_pos;
  init_BFS(players_state, source, dist, parent_dir, color);
  BFS(players_state, source, parent, parent_dir, dist, color);

  // find closest object
  vertex_t obj;
  unsigned int obj_dist = UINT_MAX;
  for (unsigned int i = 0; i < players_state->graph->num_objectives; i++) {
    if (dist[players_state->graph->objectives[i]] < obj_dist &&
        !players_state->is_objective_reached[op_id][i]) {
      obj = players_state->graph->objectives[i];
      obj_dist = dist[obj];
    }
  }
  if (obj_dist == UINT_MAX)
    obj = players_state->graph->start[op_id];

  // path
  vertex_t op_path[n];
  size_t op_path_len = 0;
  vertex_t v = obj;
  while (v != source) {
    op_path[op_path_len] = v;
    v = parent[v];
    op_path_len++;
  }
  op_path[op_path_len++] = source;

  // find best wall placement
  unsigned int obj_dist_wall = obj_dist;
  struct edge_t e[2] = {{.fr = UINT_MAX, .to = UINT_MAX},
                        {.fr = UINT_MAX, .to = UINT_MAX}};
;
  for (unsigned int i = 0; i + 1 < op_path_len; i++) {
    struct edge_t e_tmp[] = {{.fr = op_path[i], .to = op_path[i + 1]},
                             {.fr = op_path[i], .to = UINT_MAX}};

    // find e[1].to
    for (int k = csr->p[op_path[i]]; k < csr->p[op_path[i] + 1]; k++) {
      vertex_t v = csr->i[k];
      unsigned int val_v = csr->data[k];      
      if (!IS_WALL(val_v) &&
          gsl_spmatrix_uint_get(csr, v, op_path[i + 1]) != NO_EDGE) {
        e_tmp[1].to = v;
        break;
      }
    }
    if (e_tmp[1].to == UINT_MAX)  
      continue;
    printf("Ya1\n");
    printf("%d %d %d %d\n", e_tmp[0].fr, e_tmp[0].to, e_tmp[1].fr, e_tmp[1].to);
    build_wall(players_state, e_tmp[0]);
    build_wall(players_state, e_tmp[1]);
    printf("Ya2\n");

    init_BFS(players_state, source, dist, parent_dir, color);
    BFS(players_state, source, parent, parent_dir, dist, color);

    if (dist[obj] > obj_dist_wall && dist[obj] < UINT_MAX) {
      e[0] = e_tmp[0];
      e[1] = e_tmp[1];
      obj_dist_wall = dist[obj];
    }

    printf("Ya\n");
    // reset wall
    remove_wall(players_state, e_tmp[0]);
    remove_wall(players_state, e_tmp[1]);
  }

  free(dist);
  free(parent);
  free(parent_dir);
  free(color);

  printf("lkjfdsq %d %d\n", obj_dist_wall, obj_dist);
  printf("waterloo %d (%d %d %d %d)\n", players_state->prev_move.t, players_state->prev_move.e[0].fr, players_state->prev_move.e[0].to, players_state->prev_move.e[1].fr, players_state->prev_move.e[1].to);
  // Verify acceptable edge
  if (e[1].to == UINT_MAX)
    return 0;

  // Do not place wall if not worth it
  if (abs((int)obj_dist_wall - (int)obj_dist) < WALL_DIFF)
    return 0;

  // updates world
  build_wall(players_state, e[0]);
  build_wall(players_state, e[1]);
  global_glouton(players_state, path);

  *move = (struct move_t){.t = WALL, .e = {e[0], e[1]}};
  return 1;
}
