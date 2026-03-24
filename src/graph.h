#ifndef _CORS_GRAPH_H_
#define _CORS_GRAPH_H_

#include <gsl/gsl_spblas.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <stddef.h>  // Necessary for gsl

#include "move.h"

/* Possible directions of the graph */
enum dir_t {
  NO_EDGE = 0,
  NW = 1,
  NE = 2,
  E = 3,
  SE = 4,
  SW = 5,
  W = 6,
  FIRST_DIR = NW,
  LAST_DIR = W,
  NUM_DIRS = 6,
  WALL_DIR = 7,
};

/* A mask for handling directions with a wall */
#define WALL_MASK (((unsigned int)1) << (sizeof(unsigned int) * 8 - 1))

/* A function to determine the opposite direction of a direction `d` */
static inline enum dir_t opposite_dir(enum dir_t d) {
  return (d == 0) ? 0 : (d >= 4) ? (d - 3) : (d + 3);
}

/* A function to determine the next dir in the counterclockwise order */
static inline enum dir_t next_dir(enum dir_t d) {
  return (d == 0) ? 0 : (d == 1) ? LAST_DIR : (d - 1);
}

/* The different types of graphs on which the game is played */
enum graph_type_t {
  PARALLELOGRAM = 0,
  TRIANGULAR = 1,
  CYCLIC = 2,
  HOLEY = 3,
  LINEAR = 4,
  TRIANGULAR_RANDOM = 5,
  HOLEY_RANDOM = 6,
  SPAN = 7,
};

/* A transformation from char to graph types */
static inline enum graph_type_t graph_type(char c1, char c2) {
  switch (c1) {
    case 'P':
      return PARALLELOGRAM;
    case 'T':
      return (c2 == 'R') ? TRIANGULAR_RANDOM : TRIANGULAR;
    case 'H':
      return (c2 == 'R') ? HOLEY_RANDOM : HOLEY;
    case 'C':
      return CYCLIC;
    case 'L':
      return LINEAR;
    case 'S':
      return SPAN;
    default:
      return PARALLELOGRAM;
  }
}

/* The representation of a graph */
struct graph_t {
  enum graph_type_t type;     // The type of the graph
  unsigned int num_vertices;  // Number of vertices in the graph
  unsigned int num_edges;     // Number of edges in the graph
  gsl_spmatrix_uint* t;  // Sparse matrix of size num_vertices*num_vertices,
                         // t[i][j] > 0 means there is an edge from i to j
                         // t[i][j] == E means that j is EAST of i
                         // t[i][j] == W means that j is WEST of i
                         // and so on
  vertex_t start[NUM_PLAYERS];  // Starting vertices of both players
  unsigned int num_objectives;  // Number of objectives in the graph
  vertex_t* objectives;         // Objectives of the graph
};

#endif  // _CORS_GRAPH_H_
