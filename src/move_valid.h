#include "graph.h"






//checks if the move is valid, returns 1 and if it's valid 0 if not 
int move_is_valid(struct graph_t *graph, struct move_t move, struct move_t *previous_moves);


//checks if the move is a waterloo, return 1 if it's a waterloo and 0 if not
int move_is_waterloo(struct graph_t *graph, struct move_t move);


//Takes a move made by a player that is a waterloo and returns its waterloo
struct move_t waterloo_move(struct move_t move);


