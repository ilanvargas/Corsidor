#include <stdio.h>

#include "graph_show.h"

void graph_show(struct graph_t* graph, char* file_name)
{
  FILE* fptr = fopen(file_name, "w");

  fprintf(fptr, "graph G { \n");

  size_t N = graph->t->size1;
  for (size_t i = 0; i < N; i++) {
    
    int is_show = 0;
    for (size_t j = i; j < N; j++) {
      if (gsl_spmatrix_uint_get(graph->t, i, j) > 0){
        if (!is_show) {
          is_show = 1;
          fprintf(fptr, "\t \"%ld\" -- {", i);
        }
        fprintf(fptr, "\"%ld\" ", j);
      }
    }
    if (is_show)
      fprintf(fptr, "} \n");
  }

  fprintf(fptr, "}");
  fclose(fptr);
}
