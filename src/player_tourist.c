#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "player_tourist.h"

struct players_state_t* players_state;
struct path_t* path;

char const* get_player_name() {
  return "Tourist";
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  players_state = init_players_state(player_id, graph);
  path = init_path(graph);
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  // recalculate opponent position
  unsigned int op_id = 1 - players_state->id;
  enum move_type_t op_type = previous_moves[op_id].t;
  if (op_type == MOVE || op_type == JUMP)
    players_state->op_pos = previous_moves[op_id].m;

  // calculate path
  enum move_type_t my_type = previous_moves[players_state->id].t;
  if (my_type == NO_TYPE)
    global_glouton(players_state, path);

  if (my_type == WATERLOO) {
    build_wall(players_state, previous_moves[players_state->id].e[0]);

    players_state->prev_dir = NO_EDGE;
    players_state->pos = previous_moves[players_state->id].m;
    global_glouton(players_state, path);
  }

  // follow path
  struct move_t move = go_throught_path(players_state, path);

  return move;
}

void finalize() {
  free_path(path, players_state);
  free_players_state(players_state);
}
