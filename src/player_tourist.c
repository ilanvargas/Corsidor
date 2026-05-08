#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "player_tourist.h"

struct players_state_t players_state;
struct path_t path;

char const* get_player_name() {
  return "Tourist";
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  players_state.id = player_id;
  players_state.graph = graph;
  players_state.pos = graph->start[player_id];
  players_state.op_pos = graph->start[1 - player_id];

  players_state.is_objective_reached[player_id] =
      malloc(graph->num_objectives * sizeof(int));
  players_state.is_objective_reached[1 - player_id] =
      malloc(graph->num_objectives * sizeof(int));
  memset(players_state.is_objective_reached[player_id], 0,
         graph->num_objectives * sizeof(int));
  memset(players_state.is_objective_reached[1 - player_id], 0,
         graph->num_objectives * sizeof(int));

  players_state.num_objective_to_reach = graph->num_objectives;

  // graph->num_objectives+2 because pos -> objectives -> starting point
  path.paths = malloc((graph->num_objectives + 2) * sizeof(vertex_t*));
  for (unsigned int i = 0; i < graph->num_objectives + 2; i++)
    path.paths[i] = malloc((graph->t->size1) * sizeof(vertex_t));  // one path

  path.dists = malloc((graph->num_objectives + 1) * sizeof(unsigned int));
  path.lens = malloc((graph->num_objectives + 1) * sizeof(size_t));
  path.i_objectives = malloc(graph->num_objectives * sizeof(unsigned int));
}

#include <stdio.h>
struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  unsigned int op_id = 1 - players_state.id;
  enum move_type_t op_type = previous_moves[op_id].t;
  if (op_type == MOVE || op_type == JUMP)
    players_state.op_pos = previous_moves[op_id].m;

  enum move_type_t my_type = previous_moves[players_state.id].t;
  if (my_type == NO_TYPE || my_type == WATERLOO)
    global_glouton(&players_state, &path);

  vertex_t target = path.paths[path.i][path.j];

  // go forward
  path.j++;
  if (path.j >= path.lens[path.i]) {
    path.i++;
    path.j = 0;
  }

  // jump if opponent blocks target
  // stay on path
  if (players_state.op_pos == target) {
    vertex_t jump_target = path.paths[path.i][path.j];
    path.j++;
    if (path.j >= path.lens[path.i]) {
      path.i++;
      path.j = 0;
    }
    return (struct move_t){.c = players_state.id, .t = JUMP, .m = jump_target};
  }

  return (struct move_t){.c = players_state.id, .t = MOVE, .m = target};
}

void finalize() {
  for (unsigned int i = 0; i < players_state.graph->num_objectives + 1; i++)
    free(path.paths[i]);
  free(path.paths);

  free(path.dists);
  free(path.lens);
  free(path.i_objectives);

  for (int i = 0; i < NUM_PLAYERS; i++)
    free(players_state.is_objective_reached[i]);

  // free graph
  gsl_spmatrix_uint_free(players_state.graph->t);
  free(players_state.graph->objectives);
  free(players_state.graph);

  return;
}
