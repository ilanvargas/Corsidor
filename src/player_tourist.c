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

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  if (previous_moves[1 - players_state.id].t == MOVE ||
      previous_moves[1 - players_state.id].t == JUMP)
    players_state.op_pos = previous_moves[1 - players_state.id].m;

  if (previous_moves[players_state.id].t == NO_TYPE ||
      previous_moves[players_state.id].t == WATERLOO)
    global_glouton(&players_state, &path);

  // tacking a step throught the path
  vertex_t m = path.paths[path.i][path.j++];
  if (path.j > path.lens[path.i]) {
    path.i++;
    path.j = 0;
  }

  // jump if opponent is blocking
  if (players_state.op_pos == m) {
    path.j++;
    if (path.j > path.lens[path.i]) {
      path.i++;
      path.j = 0;
    }

    return (struct move_t){
        .c = players_state.id, .t = JUMP, .m = path.paths[path.i][path.j++]};
  }

  return (struct move_t){.c = players_state.id, .t = MOVE, .m = 10};
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
