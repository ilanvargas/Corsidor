#include "graph.h"

int move_is_valid(struct graph_t* graph,
                  struct move_t move,
                  struct move_t* previous_moves,
                  vertex_t positions[2]);

// checks if the move is a waterloo, return 1 if it's a waterloo and 0 if not
int move_is_waterloo(struct graph_t* graph,
                     struct move_t move,
                     vertex_t positions[2]);

// Takes a move made by a player that is a waterloo and returns its waterloo
struct move_t waterloo_move(struct move_t move,
                            struct graph_t* graph,
                            vertex_t positions[2]);
