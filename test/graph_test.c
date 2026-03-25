#include <stdio.h>
#include <stdlib.h>

#include "graph_show.h"

gsl_spmatrix_uint* matrix2gsl(size_t n, unsigned int mat[n][n])
{
  gsl_spmatrix_uint* t = gsl_spmatrix_uint_alloc(n, n);
  
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      gsl_spmatrix_uint_set(t, i, j, mat[i][j]);
    }
  }

  return t;
}

void test_graph_show()
{
  printf("%s\n", __func__);

  vertex_t mat[4][4] = {
    {0, 3, 4, 0},
    {6, 0, 0, 4},
    {1, 0, 0, 3},
    {0, 1, 6, 0}
  };
  vertex_t obj[] = {2};
  struct graph_t g = {
    .num_vertices = 4,
    .num_edges = 4,
    .t = matrix2gsl(4, mat),
    .start = {0, 1},
    .num_objectives = 1,
    .objectives = obj
  };

  graph_show(&g, "test1.dot");
  printf("File created as test1.dot\n");
}

int main()
{
  test_graph_show();
  
  return EXIT_SUCCESS;
}
