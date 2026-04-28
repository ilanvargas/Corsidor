#include <stdio.h>

#include "graph_impl.h"
#include "move_valid.h"
#include "test.h"

struct test_t {
  char* title;
  struct move_t move;
  vertex_t positions[2];
  vertex_t prev_positions[2];
  int sol;
};

void test_move_valid() {
  struct graph_t graph = graph_init(9, PARALLELOGRAM);

  size_t test_len = 15
  struct test_t tests[] = {
    {.title = "BLACK dist 1 dir E",
     .move = {.t = MOVE, .c = BLACK, .m = 1},
       .positions = {0, 80},
       .sol = 1},
    {.title = "BLACK dist 1 dir S",
     .move = {.t = MOVE, .c = BLACK, .m = 9},
       .positions = {0, 80},
       .sol = 1},
    {.title = "WHITE dist 1 dir W",
     .move = {.t = MOVE, .c = WHITE, .m = 79},
       .positions = {0, 80},
       .sol = 1},
    {.title = "BLACK previous E, same dir dist 2, max=3",
       .move = {.t = MOVE, .c = BLACK, .m = 3},
       .positions = {1, 80},
       .prev_position = {0, 80},
       .sol = 1},
    {.title = "BLACK previous E, same dir dist 3, max=3",
     .move = {.t = MOVE, .c = BLACK, .m = 4},
       .positions = {1, 80},
       .prev_position = {0, 80},
       .sol = 1},
    {.title = "BLACK previous E, dir 60° into S distance 2, max=2",
       .move = {.t = MOVE, .c = BLACK, .m = 19},
       .positions = {1, 80},
       .prev_position = {0, 80},
       .sol = 1},
    {.title = "WHITE previous W, same dir dist 3, max=3",
     .move = {.t = MOVE, .c = WHITE, .m = 76},
       .positions = {0, 79},
       .prev_position = {0, 80},
       .sol = 1},
    {.title = "WHITE previous W, dir 60° into N dist 2, max=2",
     .move = {.t = MOVE, .c = WHITE, .m = 61},
       .positions = {0, 79},
       .prev_position = {0, 80},
       .sol = 1},
    // --- non valid cases ---
    {.title = "Not moving",
     .move = {.t = MOVE, .c = BLACK, .m = 0},
       .positions = {0, 80},
       .sol = 0},
    {.title = "Unreacheable vertex",
     .move = {.t = MOVE, .c = BLACK, .m = 10},
       .previous_moves = no_prev,
       .positions = {0, 80},
       .sol = 0},
    {.title = "Illegal >60° 2 dist move",
     .move = {.t = MOVE, .c = BLACK, .m = 10},
       .positions = {11, 80},
       .prev_position = {10, 80},
       .sol = 0},
    {.title = "BLACK previous E, same dir dist 4",
     .move = {.t = MOVE, .c = BLACK, .m = 4},
       .positions = {1, 80},
       .prev_position = {0, 80},
       .sol = 0},
      {.title = "BLACK previous E, dir 60° into S distance 3",
       .move = {.t = MOVE, .c = BLACK, .m = 28},
       .positions = {1, 80},
       .prev_position = {0, 80},
       .sol = 0},
      {.title = "Case occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 1},
       .positions = {0, 1},
       .sol = 0},
      {.title = "Long travel occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 3},
       .previous_moves = prev_black_e,
       .positions = {1, 2},
       .prev_position = {0, 2},
       .sol = 0},
  };
  for (size_t i = 0; i < tests_len; i++) {
    
  }
}
