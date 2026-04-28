#include "player.h"

char const* p_name = "GOAT";
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


//erreur
struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  if (previous_moves[p_id].t == NO_TYPE){
    return (struct move_t){.c = p_id, .t = MOVE, .m = p_pos + 1};
  }
  return (struct move_t){.c = p_id, .t = MOVE, .m = previous_moves[p_id].m + 1};
}

void finalize() {
  return;
}
