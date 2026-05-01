#ifndef __MOVE_CHECK_H__
#define __MOVE_CHECK_H__

#include <limits.h>

#include "graph_impl.h"

#define MAX_DIST 3
#define MAX_DIST_60_DEG 2

#define INVALID_VERTEX -1
#define INVALID_EDGE \
  (struct edge_t) {  \
    -1, -1           \
  }

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

/** Verify is move is waterloo
 * If true returns 1 and modify move into waterloo type
 * Else returns 0 and does nothing
 * ARGS:
 * - move:    struct move_t the move to convert
 * - graph:   struct graph_t the game graph
 * - position:array of vertex the players positions
 * RETURN:
 * - boolean value
 */
int convert_waterloo(struct graph_t* graph,
                     struct move_t* move,
                     vertex_t positions[NUM_PLAYERS]);

#endif  //__MOVE_CHECK_H__
