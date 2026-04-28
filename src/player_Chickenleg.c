#include "player.h"
#include "move_valid.h"

char const* p_name = "Chickenleg";
unsigned int p_id;
struct graph_t p_graph;
vertex_t p_pos;

char const* get_player_name() {
  return p_name;
}

void initialize(unsigned int player_id, struct graph_t* graph) {
  p_id = player_id;
  p_graph = *graph;
  p_pos = p_graph.start[p_id];
}


struct move_t move_est(const struct move_t previous_moves[NUM_PLAYERS]){
  return (struct move_t){.c = p_id, .t = MOVE, .m = previous_moves[p_id].m + 1};
}

struct move_t move_west(const struct move_t previous_moves[NUM_PLAYERS]){
  return (struct move_t){.c = p_id, .t = MOVE, .m = previous_moves[p_id].m - 1};
}

struct move_t move_jump(const struct move_t previous_moves[NUM_PLAYERS]){
  if (previous_moves[1 - p_id].m == previous_moves[p_id].m + 1){
    return (struct move_t){.c = p_id, .t = JUMP, .m = previous_moves[p_id].m + 2};
  } else  if (previous_moves[1 - p_id].m == previous_moves[p_id].m - 1){
    return (struct move_t){.c = p_id, .t = JUMP, .m = previous_moves[p_id].m - 2};
  }
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  vertex_t positions[2];
  positions[0] = previous_moves[0].m;
  positions[1] = previous_moves[1].m;
  if (previous_moves[p_id].t == NO_TYPE){
    return (struct move_t){.c = p_id, .t = MOVE, .m = p_pos - 1};
  }
  struct move_t move = move_west(previous_moves);
  if (move_is_valid(&p_graph, move, previous_moves, positions)){
    return move;
  } else {
    move = move_est(previous_moves);
    if (move_is_valid(&p_graph, move, previous_moves, positions)){
      return move;
    } else {
      return move_jump(previous_moves);
    }
  }
}

void finalize() {
  return;
}