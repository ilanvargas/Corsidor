#include "graph.h"

/** Checks if the move is valide */
int move_is_valid(struct graph_t* graph,
                  struct move_t move,
                  vertex_t positions[2],
                  vertex_t prev_positions[2]);

/** Checks if the move  waterloo */
int move_is_waterloo(struct graph_t* graph,
                     struct move_t move,
                     vertex_t positions[2]);

/** Converts a waterloo move into waterloo type move
 * ARGS:
 * - move: struct move_t the move to convert
 * - graph: struct graph_t the game graph
 * - position: array of vertex the players positions
 * RETURN:
 * - waterloo type move
 * PRECOND:
 * - move is waterloo
 */
struct move_t waterloo_move(struct move_t move,
                            struct graph_t* graph,
                            vertex_t positions[2]);
