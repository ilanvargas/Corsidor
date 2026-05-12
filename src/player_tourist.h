#ifndef __PLAYER_TOURIST_H__
#define __PLAYER_TOURIST_H__

#include "player.h"

#define MAX_DIST_STRAIGHT 3
#define MAX_DIST_60 2

#define IS_WALL(val) (((val) & WALL_MASK) != NO_EDGE)

// diference done by placing a wall needed to place it
#define WALL_DIFF 4

// game players state attributes
struct players_state_t {
  unsigned int id;          // player color
  struct graph_t* graph;    // game graph for player
  vertex_t pos;             // position player
  vertex_t op_pos;          // position opponent
  enum dir_t prev_dir;      // previous dir taken
  struct move_t prev_move;  // previous move
  int num_walls;            // number of walls left

  int* is_objective_reached[NUM_PLAYERS];  // reached objectives for each player
  unsigned int num_objective_to_reach;     // number of objectives to reach
};

// path defintion
struct path_t {
  vertex_t** paths;            // one path = path from a objective to an other
  unsigned int* dists;         // distance of each path
  size_t* lens;                // size of each path
  unsigned int* i_objectives;  // index objectives in order reached by path
  unsigned int total_dist;     // dist total path
  vertex_t origin_source;      // where was the palyer when the path was created

  unsigned int i;  // path taking currently
  size_t j;        // step in path taking currently
};

/** Creates player_state_t based on graph */
struct players_state_t* init_players_state(unsigned int player_id,
                                           struct graph_t* graph);

/** Free player_state_t */
void free_players_state(struct players_state_t* players_state);

/** Creates a struct path_t based on graph*/
struct path_t* init_path(struct graph_t* graph);

/* Free path */
void free_path(struct path_t* path, struct players_state_t* players_state);

/** Algorithm to carve a path from source throught all objectives
 * From p_pos creates path to nearest objectives then to nearest objective so on
 * Each path is a array in paths
 * Uses BFS
 * ARGS:
 * - struct players_state_t players_state: actual player state;
 * - struct path_t path: the path to modify;
 * RETURN:
 *  modifies struct path_t creating the path
 *  resets path.i path.j
 */
void global_glouton(struct players_state_t* players_state, struct path_t* path);

/** Advances throught path and returns a move
 * Can modify move if opponent blocking
 * Can recalculate path
 * Updates path.i path.j
 */
struct move_t go_throught_path(struct players_state_t* players_state,
                               struct path_t* path);

/** Block edge e (one)
 *  Builds half of the walln(2 edges)
 */
void build_wall(struct players_state_t* players_state, struct edge_t e);

/* Unblocks one edge */
void remove_wall(struct players_state_t* players_state, struct edge_t e);

/* Decides and places wall
 * If decides to place wall then returns 1 and modifies move
 * Else returns 0
 */
int wall_move(struct players_state_t* players_state,
              struct path_t* path,
              struct move_t* move);

#endif  // __PLAYER_TOURIST_H__
