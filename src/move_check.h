#ifndef __MOVE_CHECK_H__
#define __MOVE_CHECK_H__

#include "graph.h"

/** Checks if a move is valide
 *  Changes p_walls[NUM_PLAYERS] if wall move
 *  ARGS:
 *  - struct graph_t* graph: game graph
 *  - struct move_t* move: move to verify
 *  - vertex_t positions[2]: players postions, postions[PLAYER_COLOR] = position
 * of PLAYER_COLOR
 *  - vertex_t prev_positions[2]: previous player postions, same logic
 *  - int p_walls[2]: number of walls left for players, same logic
 * */
int move_is_valid(struct graph_t* graph,
                  struct move_t* move,
                  vertex_t p_pos[NUM_PLAYERS],
                  vertex_t p_prev_pos[NUM_PLAYERS],
                  int p_walls[NUM_PLAYERS]);

/** Checks if the move is waterloo */
int move_is_waterloo(struct graph_t* graph,
                     struct move_t move,
                     vertex_t positions[NUM_PLAYERS]);

/** Converts a waterloo verifing move into waterloo type move
 * ARGS:
 * - move: struct move_t the move to convert
 * - graph: struct graph_t the game graph
 * - position: array of vertex the players positions
 * RETURN:
 * - waterloo type move
 * PRECOND:
 * - move is waterloo
 */
struct move_t convert_waterloo(struct move_t move,
                               struct graph_t* graph,
                               vertex_t positions[NUM_PLAYERS]);

#endif  //__MOVE_CHECK_H__
