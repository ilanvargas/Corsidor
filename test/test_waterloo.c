#include <stdio.h>
#include <stdlib.h>

#include "graph_impl.h"
#include "move_check.h"
#include "test.h"

struct test_t {
  char* title;
  struct move_t move;
  vertex_t p_pos[NUM_PLAYERS];
  int sol;
};

void exec_tests(struct test_t* tests, size_t tests_len, struct graph_t* graph) {
  for (size_t i = 0; i < tests_len; i++) {
    printf("\t %s: ", tests[i].title);
    int res = convert_waterloo(graph, &tests[i].move, tests[i].p_pos);
    CMP_INT(res, tests[i].sol)
    printf("\n");
  }
}

void test_move_waterloo() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);
  struct edge_t wall[2] = {{.fr = 10, .to = 11}, {.fr = 10, .to = 19}};
  build_wall(graph, wall);

  size_t move_test_len = 5;
  struct test_t move_test[] = {
      {.title = "No walls",
       .move = {.t = MOVE, .c = BLACK, .m = 1},
       .p_pos = {0, 80},
       .sol = 0},
      {.title = "Dist 3 no walls",
       .move = {.t = MOVE, .c = WHITE, .m = 76},
       .p_pos = {0, 79},
       .sol = 0},
      {.title = "Wall on first step",
       .move = {.t = MOVE, .c = BLACK, .m = 11},
       .p_pos = {10, 80},
       .sol = 1},
      {.title = "Wall on second step",
       .move = {.t = MOVE, .c = BLACK, .m = 19},
       .p_pos = {1, 80},
       .sol = 1},
      {.title = "Wall on third step",
       .move = {.t = MOVE, .c = BLACK, .m = 10},
       .p_pos = {13, 80},
       .sol = 1},
  };

  exec_tests(move_test, move_test_len, graph);
  graph_free(graph);

  COMPLETE
}

void test_jump_waterloo() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);
  struct edge_t wall[2] = {{.fr = 10, .to = 11}, {.fr = 10, .to = 19}};
  build_wall(graph, wall);

  size_t jump_test_len = 4;
  struct test_t jump_test[] = {
      {.title = "JUMP wall between player and oponent",
       .move = {.t = JUMP, .c = BLACK, .m = 28},
       .p_pos = {10, 19},
       .sol = 1},
      {.title = "JUMP wall oponent and landing",
       .move = {.t = JUMP, .c = BLACK, .m = 19},
       .p_pos = {1, 10},
       .sol = 1},
      {.title = "2 walls",
       .move = {.t = JUMP, .c = BLACK, .m = 19},
       .p_pos = {10, 11},
       .sol = 1},
      {.title = "No walls",
       .move = {.t = JUMP, .c = BLACK, .m = 9},
       .p_pos = {1, 10},
       .sol = 0},
  };

  exec_tests(jump_test, jump_test_len, graph);
  graph_free(graph);

  COMPLETE
}

void test_wall_waterloo() {
  struct graph_t* graph = graph_init(9, PARALLELOGRAM);
  struct edge_t wall[2] = {{.fr = 10, .to = 11}, {.fr = 10, .to = 19}};
  build_wall(graph, wall);

  size_t wall_test_len = 3;
  struct test_t wall_test[] = {
      {.title = "WALL on existing wall same edges",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 19}}},
       .p_pos = {0, 80},
       .sol = 1},
      {.title = "WALL sharing one edge with existing wall",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 10, .to = 11}, {.fr = 10, .to = 2}}},
       .p_pos = {0, 80},
       .sol = 1},
      {.title = "WALL on free edges",
       .move = {.t = WALL,
                .c = BLACK,
                .e = {{.fr = 40, .to = 41}, {.fr = 40, .to = 49}}},
       .p_pos = {0, 80},
       .sol = 0},
  };

  exec_tests(wall_test, wall_test_len, graph);
  graph_free(graph);

  COMPLETE
}

int main() {
  RUN(test_move_waterloo())
  RUN(test_jump_waterloo())
  RUN(test_wall_waterloo())

  return EXIT_SUCCESS;
}
