#include "move_valid.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

// checks if two directions are consecutive
int are_consecutive_dirs(enum dir_t d1, enum dir_t d2) {
  if (d1 == NO_EDGE || d2 == NO_EDGE) return 0;
  int diff = abs(d1 - d2);
  return diff == 1 || diff == 5;
}

// gets direction between 2 edges
enum dir_t get_direction(vertex_t fr, vertex_t to, struct graph_t *graph) {
  unsigned int val = gsl_spmatrix_uint_get(graph->t, fr, to);
  return (enum dir_t)val;
}

// check if position is valid
int is_valid_position(vertex_t pos, struct graph_t *graph) {
  return pos < graph->num_vertices;
}

// check if edge exists, i.e: not blocked by wall and actually exists in the graph
int edge_exists(vertex_t fr, vertex_t to, struct graph_t *graph) {
  return gsl_spmatrix_uint_get(graph->t, fr, to) != NO_EDGE;
}

static int is_adjacent(vertex_t a, vertex_t b, struct graph_t *graph) {
  return edge_exists(a, b, graph);
}

int dirs_are_60_degrees_apart(enum dir_t d1, enum dir_t d2) {
  if (d1 == NO_EDGE || d2 == NO_EDGE) return 0;
  return are_consecutive_dirs(d1, d2) || are_consecutive_dirs(d2, d1);
}

// Helper to check if path is straight line
// Returns 1 if path from start to end, that has a distance of dist, maintains the same direction dir
int is_straight_path(vertex_t start, vertex_t end, struct graph_t *graph, enum dir_t dir, unsigned int dist) {
  vertex_t temp = start;
  int steps = 0;
  while (temp != end && steps <= dist) {
    int found = 0;
    for (vertex_t neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
      if (edge_exists(temp, neighbor, graph) && get_direction(temp, neighbor, graph) == dir) {
        temp = neighbor;
        found = 1;
        break;
      }
    }
    if (!found) return 0; // No path in the right direction (possibly blocked by wall)
    steps++;
  }
  return temp == end; // Check if we reached the end within the distance limit
}

// check if there's a path from start to any of the targets using BFS
int has_path(vertex_t start, vertex_t *targets, int num_targets, struct graph_t *graph) {
  int visited[graph->num_vertices];
  memset(visited, 0, sizeof(visited));
  vertex_t queue[graph->num_vertices];
  int read = 0, write = 0;
  queue[write++] = start;
  visited[start] = 1;

  while (read < write) {
    vertex_t current = queue[read++];
    for (vertex_t neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
      if (edge_exists(current, neighbor, graph) && !visited[neighbor]) {
        visited[neighbor] = 1;
        queue[write++] = neighbor;
      }
    }
  }

  for (int i = 0; i < num_targets; i++) {
    if (!visited[targets[i]]) return 0;
  }
  return 1;
}

unsigned int distance(vertex_t current_pos, enum dir_t dir, vertex_t target_pos, struct graph_t *graph) {
  int dist = 0;
  vertex_t temp = current_pos;
  while (temp != target_pos) {
    for (vertex_t neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
      if (edge_exists(temp, neighbor, graph) && get_direction(temp, neighbor, graph) == dir) {
        temp = neighbor;
        dist++;
        break;
      }
    }
    if (temp == current_pos){
      printf("on n'a pas progressé\n");
      return 0; // No path in the right direction (possibly blocked by wall)
    }
  }
  return dist;
}

int wall_on_path(vertex_t start, enum dir_t dir, vertex_t end, struct graph_t *graph) {
  vertex_t temp = start;
  while (temp != end) {
    int found = 0;
    for (vertex_t neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
      if (edge_exists(temp, neighbor, graph) && get_direction(temp, neighbor, graph) == dir) {
        temp = neighbor;
        found = 1;
        break;
      }
    }
    if (!found) return 1; // Blocked
  }
  return 0; // Not blocked
}

int move_is_valid(struct graph_t *graph, struct move_t move, struct move_t *previous_moves, vertex_t positions[2]) {
  if (move.c != BLACK && move.c != WHITE) return 0;
  if (move.t == NO_TYPE) return 0;

  vertex_t current_pos = positions[move.c];
  vertex_t opponent_pos = positions[1 - move.c];
  vertex_t previous_pos = current_pos; // NEED TO TRACK PREVIOUS POSITION FOR 60 DEGREE CHECK

  unsigned int max_dist_straight = 3;
  unsigned int max_dist_60 = 2;


  if (move.t == MOVE) {
    enum dir_t move_dir = get_direction(current_pos, move.m, graph);
    if (!is_valid_position(move.m, graph)) return 0;
    if (move.m == opponent_pos || move.m == current_pos) return 0;
    unsigned int dist = distance(current_pos, get_direction(current_pos, move.m, graph), move.m, graph);
    if (is_straight_path(current_pos, move.m, graph, move_dir, dist) && dist <= max_dist_straight) return 1;
    if (dirs_are_60_degrees_apart(get_direction(previous_pos, current_pos, graph), get_direction(current_pos, move.m, graph)) && dist <= max_dist_60) return 1;
    if (is_adjacent(current_pos, move.m, graph)) return 1;
    return 0;

  } else if (move.t == JUMP) {
    if (!is_valid_position(move.m, graph)) return 0;
    if (move.m == current_pos || move.m == opponent_pos) return 0;
    if (!is_adjacent(current_pos, opponent_pos, graph)) return 0;
    if (!is_adjacent(opponent_pos, move.m, graph)) return 0;
    return 1;
  } else if (move.t == WALL) {
    if (move.e[0].fr != move.e[1].fr) return 0;
    if (move.e[0].to == move.e[1].to) return 0;
    if (!is_valid_position(move.e[0].fr, graph) || !is_valid_position(move.e[0].to, graph) ||
        !is_valid_position(move.e[1].fr, graph) || !is_valid_position(move.e[1].to, graph)) {
      return 0;
    }
    enum dir_t d1 = get_direction(move.e[0].fr, move.e[0].to, graph);
    enum dir_t d2 = get_direction(move.e[1].fr, move.e[1].to, graph);
    if (!are_consecutive_dirs(d1, d2) && !are_consecutive_dirs(d2, d1)) {
      return 0;
    }
    if (d1 == NO_EDGE || d2 == NO_EDGE) {
      return 1;
    }
  }
  return 1;
}

int move_is_waterloo(struct graph_t *graph, struct move_t move, vertex_t positions[2]) {
  vertex_t current_pos = positions[move.c];
  vertex_t opponent_pos = positions[1 - move.c];

  if (move.t == MOVE) {
   // Check if path is blocked by wall  
    if (wall_on_path(current_pos, get_direction(current_pos, move.m, graph), move.m, graph)) return 1;
    return 0;
  } else if (move.t == JUMP) {
    if (!edge_exists(current_pos, opponent_pos, graph) || !edge_exists(opponent_pos, move.m, graph)) return 1;
    return 0;
  } else if (move.t == WALL) {
    if (!edge_exists(move.e[0].fr, move.e[0].to, graph) || !edge_exists(move.e[1].fr, move.e[1].to, graph)) return 1;
    // Check connectivity
    unsigned int orig1 = gsl_spmatrix_uint_get(graph->t, move.e[0].fr, move.e[0].to);
    unsigned int orig2 = gsl_spmatrix_uint_get(graph->t, move.e[1].fr, move.e[1].to);
    unsigned int orig1_rev = gsl_spmatrix_uint_get(graph->t, move.e[0].to, move.e[0].fr);
    unsigned int orig2_rev = gsl_spmatrix_uint_get(graph->t, move.e[1].to, move.e[1].fr);
    gsl_spmatrix_uint_set(graph->t, move.e[0].fr, move.e[0].to, NO_EDGE);
    gsl_spmatrix_uint_set(graph->t, move.e[1].fr, move.e[1].to, NO_EDGE);
    gsl_spmatrix_uint_set(graph->t, move.e[0].to, move.e[0].fr, NO_EDGE);
    gsl_spmatrix_uint_set(graph->t, move.e[1].to, move.e[1].fr, NO_EDGE);

    int connected = 1;
    for (int p = 0; p < 2; p++) {
      vertex_t targets[graph->num_objectives + 1];
      memcpy(targets, graph->objectives, sizeof(vertex_t) * graph->num_objectives);
      targets[graph->num_objectives] = graph->start[p];
      if (!has_path(positions[p], targets, graph->num_objectives + 1, graph)) {
        connected = 0;
        break;
      }
    }
    gsl_spmatrix_uint_set(graph->t, move.e[0].fr, move.e[0].to, orig1);
    gsl_spmatrix_uint_set(graph->t, move.e[1].fr, move.e[1].to, orig2);
    gsl_spmatrix_uint_set(graph->t, move.e[0].to, move.e[0].fr, orig1_rev);
    gsl_spmatrix_uint_set(graph->t, move.e[1].to, move.e[1].fr, orig2_rev);

    if (!connected) return 1;
    return 0;
  }
  return 0;
}

struct move_t waterloo_move(struct move_t move, struct graph_t *graph, vertex_t positions[2]) {
  vertex_t current_pos = positions[move.c];
  if (move.t == MOVE) {
    // For MOVE: Find the blocking edge on the path from current_pos to move.m
    vertex_t temp = current_pos;  // Current position in path traversal
    vertex_t last = current_pos;  // Last valid position before blockage
    enum dir_t dir = NO_EDGE;     // Direction of movement (initialized later)

    // Traverse the path step by step to find where it's blocked
    while (temp != move.m) {
      int found = 0;  // Flag to indicate if we found a valid next step

      // Try all possible directions to find the next vertex on the path
      for (enum dir_t d = NW; d <= W; d++) {
        for (vertex_t neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
          // Check if there's an edge from temp to neighbor in direction d
          if (edge_exists(temp, neighbor, graph) && get_direction(temp, neighbor, graph) == d) {
            // First move: record the direction
            if (dir == NO_EDGE) dir = d;

            // Update positions: last becomes temp, temp becomes neighbor
            last = temp;
            temp = neighbor;
            found = 1;
            break;  // Found the next step, exit inner loop
          }
        }
        if (found) break;  // Exit direction loop if we found a move
      }

      // If no valid neighbor found in any direction, path is blocked
      if (!found) {
        // Create WATERLOO move with the blocking edge (last -> temp)
        struct edge_t blocking = {last, temp};
        return (struct move_t){.c = move.c, .t = WATERLOO, .m = last, .e = {blocking, {0, 0}}};
      }
    }

    // If we reach here, the path was somehow valid (shouldn't happen for invalid moves)
    return move;

  } else if (move.t == JUMP) {
    // For JUMP: The blocking edge is between current_pos and opponent's position
    // (since jump requires adjacency to opponent)
    if (!edge_exists(current_pos, positions[1 - move.c], graph)) {
      struct edge_t blocking = {current_pos, positions[1 - move.c]};
      return (struct move_t){.c = move.c, .t = WATERLOO, .m = current_pos, .e = {blocking, {0, 0}}};
    }
    if (!edge_exists(positions[1 - move.c], move.m, graph)) {
      struct edge_t blocking = {positions[1 - move.c], move.m};
      return (struct move_t){.c = move.c, .t = WATERLOO, .m = current_pos, .e = {blocking, {0, 0}}};
    }

  } else if (move.t == WALL) {
    // For WALL: Check which edge of the wall doesn't exist in the graph
    if (!edge_exists(move.e[0].fr, move.e[0].to, graph)) {
      // First edge doesn't exist - that's the blocking one
      return (struct move_t){.c = move.c, .t = WATERLOO, .m = -1, .e = {move.e[0], {0, 0}}};
    } else if (!edge_exists(move.e[1].fr, move.e[1].to, graph)) {
      // Second edge doesn't exist - that's the blocking one
      return (struct move_t){.c = move.c, .t = WATERLOO, .m = -1, .e = {move.e[1], {0, 0}}};
    } else {
      // Both edges exist, but wall is invalid because it stops opponent from reaching objectives.
      return (struct move_t){.c = move.c, .t = WATERLOO, .m = -1, .e = {{0, 0}, {0, 0}}};
    }
  }

  // Fallback: return original move unchanged
  return move;
}