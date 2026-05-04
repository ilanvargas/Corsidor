#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "graph_impl.h"
#include "move_check.h"
#include "test.h"

// --- Test helper functions ---

int path_dir_diff(struct graph_t* graph,
                  vertex_t prev,
                  vertex_t fr,
                  vertex_t to,
                  enum dir_t* dir_go);

void test_helper_path_dir_diff() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);
  enum dir_t dir_go;

  printf("\t Straight: ");
  CMP_INT(path_dir_diff(graph, 0, 1, 2, &dir_go), 0)
  CMP_INT(dir_go, W)
  CMP_INT(path_dir_diff(graph, 1, 28, 73, &dir_go), 0)
  CMP_INT(dir_go, SE)
  printf("\n");

  printf("\t 60 degrees: ");
  CMP_INT(path_dir_diff(graph, 0, 1, 11, &dir_go), 1)
  CMP_INT(dir_go, SW)
  CMP_INT(path_dir_diff(graph, 30, 20, 19, &dir_go), 1)
  CMP_INT(dir_go, E)
  printf("\n");

  printf("\t More angular: ");
  CMP_INT(path_dir_diff(graph, 0, 1, 0, &dir_go), 3)
  CMP_INT(dir_go, E)
  CMP_INT(path_dir_diff(graph, 1, 28, 19, &dir_go), 3)
  CMP_INT(dir_go, NW)
  printf("\n");

  graph_free(graph);
}

// --- Tests main functions ---

struct test_t {
  char* title;
  struct move_t move;
  vertex_t p_pos[NUM_PLAYERS];
  vertex_t p_prev_pos[NUM_PLAYERS];
  int p_walls[NUM_PLAYERS];
  int sol;
};

void exec_tests(struct test_t* tests, size_t tests_len, struct graph_t* graph) {
  for (size_t i = 0; i < tests_len; i++) {
    printf("\t %s: ", tests[i].title);
    int res = move_is_valid(graph, &tests[i].move, tests[i].p_pos,
                            tests[i].p_prev_pos, tests[i].p_walls);
    CMP_INT(res, tests[i].sol)
    printf("\n");
  }
}

void test_move_move_is_valid() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);

  size_t move_test_len = 16;
  struct test_t move_test[] = {
      {.title = "BLACK dist 1 dir W",
       .move = {.t = MOVE, .c = BLACK, .m = 1},
       .p_pos = {0, 80},
       .p_prev_pos = {UINT_MAX, UINT_MAX},
       .sol = 1},
      {.title = "BLACK dist 1 dir SE",
       .move = {.t = MOVE, .c = BLACK, .m = 9},
       .p_pos = {0, 80},
       .p_prev_pos = {UINT_MAX, UINT_MAX},
       .sol = 1},
      {.title = "WHITE dist 1 dir E",
       .move = {.t = MOVE, .c = WHITE, .m = 79},
       .p_pos = {0, 80},
       .p_prev_pos = {UINT_MAX, UINT_MAX},
       .sol = 1},
      {.title = "BLACK previous W, same dir dist 2, max=3",
       .move = {.t = MOVE, .c = BLACK, .m = 3},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 1},
      {.title = "BLACK previous W, same dir dist 3, max=3",
       .move = {.t = MOVE, .c = BLACK, .m = 4},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 1},
      {.title = "BLACK previous W, dir 60° into SW distance 2, max=2",
       .move = {.t = MOVE, .c = BLACK, .m = 21},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 1},
      {.title = "WHITE previous E, same dir dist 3, max=3",
       .move = {.t = MOVE, .c = WHITE, .m = 76},
       .p_pos = {0, 79},
       .p_prev_pos = {0, 80},
       .sol = 1},
      {.title = "WHITE previous E, dir 60° into NE dist 2, max=2",
       .move = {.t = MOVE, .c = WHITE, .m = 59},
       .p_pos = {0, 79},
       .p_prev_pos = {0, 80},
       .sol = 1},
      // --- non valid cases ---
      {.title = "Not moving",
       .move = {.t = MOVE, .c = BLACK, .m = 0},
       .p_pos = {0, 80},
       .sol = 0},
      {.title = "Unreacheable vertex",
       .move = {.t = MOVE, .c = BLACK, .m = 21},
       .p_pos = {0, 80},
       .sol = 0},
      {.title = "Illegal >60° 2 dist move",
       .move = {.t = MOVE, .c = BLACK, .m = 22},
       .p_pos = {20, 80},
       .p_prev_pos = {11, 80},
       .sol = 0},
      {.title = "BLACK previous W, same dir dist 4",
       .move = {.t = MOVE, .c = BLACK, .m = 5},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 0},
      {.title = "BLACK same dir no straight line",
       .move = {.t = MOVE, .c = BLACK, .m = 12},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 0},
      {.title = "BLACK previous W, dir 60° into SW distance 3",
       .move = {.t = MOVE, .c = BLACK, .m = 31},
       .p_pos = {1, 80},
       .p_prev_pos = {0, 80},
       .sol = 0},
      {.title = "Case occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 1},
       .p_pos = {0, 1},
       .sol = 0},
      {.title = "Long travel occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 3},
       .p_pos = {1, 2},
       .p_prev_pos = {0, 2},
       .sol = 0},
  };

  exec_tests(move_test, move_test_len, graph);
  graph_free(graph);

  COMPLETE
}

void test_jump_move_is_valid() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);

  size_t jump_test_len = 8;
  struct test_t jump_test[] = {
      {.title = "BLACK jumps over WHITE to E",
       .move = {.t = JUMP, .c = BLACK, .m = 3},
       .p_pos = {1, 2},
       .sol = 1},
      {.title = "BLACK jumps over WHITE to S",
       .move = {.t = JUMP, .c = BLACK, .m = 11},
       .p_pos = {1, 2},
       .sol = 1},
      {.title = "WHITE jumps over BLACK to W",
       .move = {.t = JUMP, .c = WHITE, .m = 77},
       .p_pos = {78, 79},
       .sol = 1},
      {.title = "WHITE jumps over BLACK to N of BLACK",
       .move = {.t = JUMP, .c = WHITE, .m = 69},
       .p_pos = {78, 79},
       .sol = 1},
      // --- non valid cases ---
      {.title = "JUMP invalid adversary not adjacent",
       .move = {.t = JUMP, .c = BLACK, .m = 71},
       .p_pos = {0, 80},
       .sol = 0},
      {.title = "JUMP invalid landing on own position",
       .move = {.t = JUMP, .c = BLACK, .m = 1},
       .p_pos = {1, 2},
       .sol = 0},
      {.title = "JUMP invalid landing not adjacent to adversary",
       .move = {.t = JUMP, .c = BLACK, .m = 5},
       .p_pos = {1, 2},
       .sol = 0},
      {.title = "JUMP invalid landing on adversary",
       .move = {.t = JUMP, .c = BLACK, .m = 2},
       .p_pos = {1, 2},
       .sol = 0},
  };

  exec_tests(jump_test, jump_test_len, graph);
  graph_free(graph);

  COMPLETE
}

void test_wall_move_is_valid() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);

  size_t wall_test_len = 8;
  struct test_t wall_test[] = {
      {.title = "WALL valid from 10",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 20}}},
       .p_walls = {5, 5},
       .sol = 1},
      {.title = "WALL valid from 40",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 40, .to = 49}, {.fr = 40, .to = 39}}},
       .p_walls = {5, 5},
       .sol = 1},
      {.title = "WALL valid form 42",
       .move = {.t = WALL,
                .c = WHITE,
                .e = {{.fr = 50, .to = 41}, {.fr = 50, .to = 51}}},
       .p_walls = {5, 5},
       .sol = 1},
      // --- non valid cases ---
      {.title = "WALL invalid no walls left",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 19}}},
       .p_walls = {0, 5},
       .sol = 0},
      {.title = "WALL invalid different fr",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 11, .to = 19}}},
       .p_walls = {5, 5},
       .sol = 0},
      {.title = "WALL invalid non existing edge",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 15}}},
       .p_pos = {0, 80},
       .p_walls = {5, 5},
       .sol = 0},
      {.title = "WALL invalid non consecutive dirs",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 9}}},
       .p_pos = {0, 80},
       .p_walls = {5, 5},
       .sol = 0},
      {.title = "WALL invalid same edge twice",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 11}}},
       .p_pos = {0, 80},
       .p_walls = {5, 5},
       .sol = 0},
  };

  exec_tests(wall_test, wall_test_len, graph);
  graph_free(graph);

  COMPLETE
}

// --- Main ---

int main() {
  RUN(test_helper_path_dir_diff())
  RUN(test_move_move_is_valid())
  RUN(test_jump_move_is_valid())
  RUN(test_wall_move_is_valid())

  return EXIT_SUCCESS;
}
