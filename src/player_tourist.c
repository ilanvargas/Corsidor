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
  if (previous_moves[op_id].t == MOVE || previous_moves[op_id].t == JUMP) {
    players_state->op_pos = previous_moves[op_id].m;
    // update objetive_reached
    for (unsigned int k = 0; k < players_state->graph->num_objectives; k++) {
      if (players_state->graph->objectives[k] == previous_moves[op_id].m &&
          !players_state->is_objective_reached[op_id][k]) {
        players_state->is_objective_reached[op_id][k] = 1;
        break;
      }
    }
  }

  // calculate path
  if (previous_moves[players_state->id].t == NO_TYPE)
    global_glouton(players_state, path);

  if (previous_moves[players_state->id].t == WATERLOO) {
    if (players_state->prev_move.t != WALL) {
      players_state->prev_dir = NO_EDGE;
      players_state->pos = previous_moves[players_state->id].m;
      global_glouton(players_state, path);
    } else {
      remove_wall(players_state, players_state->prev_move.e[0]);
      remove_wall(players_state, players_state->prev_move.e[1]);
    }

    // if not INVALID_EDGE
    if (previous_moves[players_state->id].e[0].fr != UINT_MAX)
      build_wall(players_state, previous_moves[players_state->id].e[0]);
  }

  struct move_t move;
  if (!wall_move(players_state, path, &move))
    move = go_throught_path(players_state, path);

  players_state->prev_move = move;
  return move;
}

void finalize() {
  free_path(path, players_state);
  free_players_state(players_state);
}
