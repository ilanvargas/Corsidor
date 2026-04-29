#include <stdio.h>
#include <stdlib.h>

#include "graph_impl.h"
#include "move_check.h"
#include "test.h"

// --- Tests move type helper functions ---

int path_dir_diff(struct graph_t* graph,
                  vertex_t prev,
                  vertex_t fr,
                  vertex_t to,
                  enum dir_t* dir_go); 

void test_path_dir_diff() {
  struct graph_t graph = graph_init(9, PARALLELOGRAM);
  enum dir_t dir_go;

  printf("\t Straight: ");
  printf("%d %d -- ", path_dir_diff(&graph, 0, 1, 2, &dir_go), dir_go);
  printf("%d %d -- ", path_dir_diff(&graph, 1, 28, 73, &dir_go), dir_go);

  CMP_INT(path_dir_diff(&graph, 0, 1, 2, &dir_go), 0)
  CMP_INT(dir_go, E)
  CMP_INT(path_dir_diff(&graph, 1, 28, 73, &dir_go), 0)
  CMP_INT(dir_go, SW)
  printf("\n");

  printf("\t 60 degrees: ");
  printf("%d %d -- ", path_dir_diff(&graph, 0, 1, 10, &dir_go), dir_go);
  printf("%d %d -- ", path_dir_diff(&graph, 1, 28, 30, &dir_go), dir_go);

  CMP_INT(path_dir_diff(&graph, 0, 1, 10, &dir_go), 1)
  CMP_INT(dir_go, SW)
  CMP_INT(path_dir_diff(&graph, 1, 28, 30, &dir_go), 1)
  CMP_INT(dir_go, E)
  printf("\n");

  printf("\t More angular: ");
  printf("%d %d -- ", path_dir_diff(&graph, 0, 1, 0, &dir_go), dir_go);
  printf("%d %d -- ", path_dir_diff(&graph, 1, 28, 27, &dir_go), dir_go);

  CMP_INT(path_dir_diff(&graph, 0, 1, 0, &dir_go), 1)
  CMP_INT(dir_go, SW)
  CMP_INT(path_dir_diff(&graph, 1, 28, 27, &dir_go), 1)
  CMP_INT(dir_go, E)
  printf("\n");
}

// --- Tests main functions ---

struct test_t {
  char* title;
  struct move_t move;
  vertex_t positions[2];
  vertex_t prev_positions[2];
  int sol;
};

void exec_tests(struct test_t* tests, size_t tests_len, struct graph_t* graph) {
  for (size_t i = 0; i < tests_len; i++) {
    printf("\t %s: ", tests[i].title);
    int res = move_is_valid(graph, &tests[i].move, tests[i].positions,
                            tests[i].prev_positions);
    CMP_INT(res, tests[i].sol)
    printf("\n");
  }
}

void test_move() {
  struct graph_t graph = graph_init(9, PARALLELOGRAM);

  size_t move_test_len = 15;
  struct test_t move_test[] = {
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
       .prev_positions = {0, 80},
       .sol = 1},
      {.title = "BLACK previous E, same dir dist 3, max=3",
       .move = {.t = MOVE, .c = BLACK, .m = 4},
       .positions = {1, 80},
       .prev_positions = {0, 80},
       .sol = 1},
      {.title = "BLACK previous E, dir 60° into S distance 2, max=2",
       .move = {.t = MOVE, .c = BLACK, .m = 19},
       .positions = {1, 80},
       .prev_positions = {0, 80},
       .sol = 1},
      {.title = "WHITE previous W, same dir dist 3, max=3",
       .move = {.t = MOVE, .c = WHITE, .m = 76},
       .positions = {0, 79},
       .prev_positions = {0, 80},
       .sol = 1},
      {.title = "WHITE previous W, dir 60° into N dist 2, max=2",
       .move = {.t = MOVE, .c = WHITE, .m = 61},
       .positions = {0, 79},
       .prev_positions = {0, 80},
       .sol = 1},
      // --- non valid cases ---
      {.title = "Not moving",
       .move = {.t = MOVE, .c = BLACK, .m = 0},
       .positions = {0, 80},
       .sol = 0},
      {.title = "Unreacheable vertex",
       .move = {.t = MOVE, .c = BLACK, .m = 10},
       .positions = {0, 80},
       .sol = 0},
      {.title = "Illegal >60° 2 dist move",
       .move = {.t = MOVE, .c = BLACK, .m = 18},
       .positions = {20, 80},
       .prev_positions = {11, 80},
       .sol = 0},
      {.title = "BLACK previous E, same dir dist 4",
       .move = {.t = MOVE, .c = BLACK, .m = 4},
       .positions = {1, 80},
       .prev_positions = {0, 80},
       .sol = 0},
      {.title = "BLACK previous E, dir 60° into S distance 3",
       .move = {.t = MOVE, .c = BLACK, .m = 28},
       .positions = {1, 80},
       .prev_positions = {0, 80},
       .sol = 0},
      {.title = "Case occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 1},
       .positions = {0, 1},
       .sol = 0},
      {.title = "Long travel occupied by player",
       .move = {.t = MOVE, .c = BLACK, .m = 3},
       .positions = {1, 2},
       .prev_positions = {0, 2},
       .sol = 0},
  };

  exec_tests(move_test, move_test_len, &graph);

  COMPLETE
}

void test_jump() {
  struct graph_t graph = graph_init(9, PARALLELOGRAM);

  size_t jump_test_len = 15;
  struct test_t jump_test[] = {
      {.title = "BLACK jumps over WHITE to E",
       .move = {.t = JUMP, .c = BLACK, .m = 3},
       .positions = {1, 2},
       .sol = 1},
      {.title = "BLACK jumps over WHITE to S",
       .move = {.t = JUMP, .c = BLACK, .m = 11},
       .positions = {1, 2},
       .sol = 1},
      {.title = "WHITE jumps over BLACK to W",
       .move = {.t = JUMP, .c = WHITE, .m = 77},
       .positions = {78, 79},
       .sol = 1},
      {.title = "WHITE jumps over BLACK to N of BLACK",
       .move = {.t = JUMP, .c = WHITE, .m = 69},
       .positions = {78, 79},
       .sol = 1},
      // --- non valid cases ---
      {.title = "JUMP invalid adversary not adjacent",
       .move = {.t = JUMP, .c = BLACK, .m = 71},
       .positions = {0, 80},
       .sol = 0},
      {.title = "JUMP invalid landing on own position",
       .move = {.t = JUMP, .c = BLACK, .m = 1},
       .positions = {1, 2},
       .sol = 0},
      {.title = "JUMP invalid landing not adjacent to adversary",
       .move = {.t = JUMP, .c = BLACK, .m = 5},
       .positions = {1, 2},
       .sol = 0},
      {.title = "JUMP invalid landing on adversary",
       .move = {.t = JUMP, .c = BLACK, .m = 2},
       .positions = {1, 2},
       .sol = 0},
  };

  exec_tests(jump_test, jump_test_len, &graph);

  COMPLETE
}

// --- Main ---

int main() {
  RUN(test_path_dir_diff())
  RUN(test_move())

  return EXIT_SUCCESS;
}
