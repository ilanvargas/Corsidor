#include "player.h"

struct player_t = {
  char* player_name;
  unsigned int player_id;
  vertex_t player_pos;
  unsigned int nb_walls;
  struct graph_t* graph;
};

struct player_t p;

void initialize(unsigned int player_id, struct graph_t* graph)
{
  p.player_name = "P";
  p.player_id = player_id;
  p.player_pos = graph.start[player_id];
  p.graph = graph;
  p.nb_walls = graph.vertices/16
}

struct move_t play(const struct move_t previous_moves[NUM_PLAYERS]) {
  return MOVE;
}

void finalize()
{
  return;
}



