#include <stdio.h>

#include "graph_impl.h"
#include "move_valid.h"
#include "test.h"

struct test_t {
  struct move_t move;
  struct move_t* previous_moves;
  vertex_t positions[2];
  int sol
};

void test_move() {
  struct test_t test = {
    {
      {}
    }
  };
}
