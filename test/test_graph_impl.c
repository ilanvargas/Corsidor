#include <stdio.h>
#include <stdlib.h>

#include "graph_impl.h"
#include "test.h"

gsl_spmatrix_uint* matrix2gsl(size_t n, unsigned int mat[n][n]) {
  gsl_spmatrix_uint* tmp = gsl_spmatrix_uint_alloc(n, n);

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      if (mat[i][j] != 0)
        gsl_spmatrix_uint_set(tmp, i, j, mat[i][j]);
    }
  }

  gsl_spmatrix_uint* t = gsl_spmatrix_uint_crs(tmp); 
  free(tmp);
  return t;
}

void test_graph_show() {
  vertex_t mat[4][4] = {{0, 3, 4, 0}, {6, 0, 0, 4}, {1, 0, 0, 3}, {0, 1, 6, 0}};
  vertex_t obj[] = {2};
  struct graph_t g = {.num_vertices = 4,
                      .num_edges = 4,
                      .t = matrix2gsl(4, mat),
                      .start = {0, 1},
                      .num_objectives = 1,
                      .objectives = obj};

  graph_show(&g, "test1.dot");
  fprintf(stderr, "File created as test1.dot\n");

  COMPLETE
}

void test_graph_init() {
  printf("Test for parrallelogram graph m=9\n");

  struct graph_t* g_parallelogram_m9 = graph_init(9, PARALLELOGRAM);
  graph_show(g_parallelogram_m9, "parallelogram_m9_test.dot");
  fprintf(stderr, "File created as parallelogram_m9_test.dot\n");
  graph_free(g_parallelogram_m9);

  printf("Test for parrallelogram graph m=5\n");

  struct graph_t* g_parallelogram_m5 = graph_init(5, PARALLELOGRAM);
  graph_show(g_parallelogram_m5, "parallelogram_m5_test.dot");
  fprintf(stderr, "File created as parallelogram_m5_test.dot\n");
  graph_free(g_parallelogram_m5);

  printf("Test for triangular graph\n");

  struct graph_t* g_triangular = graph_init(5, TRIANGULAR);
  graph_show(g_triangular, "triangular_test.dot");
  fprintf(stderr, "File created as triangular_test.dot\n");
  graph_free(g_triangular);

  COMPLETE
}

int main() {
  RUN(test_graph_show())
  RUN(test_graph_init())

  return EXIT_SUCCESS;
}
