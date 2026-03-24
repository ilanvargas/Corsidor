#include <stdio.h>

#include "graph_show.h"

void graph_style(struct graph_t* graph, FILE* fptr)
{
  fprintf(fptr, "\t node [shape=circle, style=filled, width=0.2, label=\"\"]; \n");

  fprintf(fptr, "\t %d [fillcolor=white]; \n", graph->start[WHITE]);
  fprintf(fptr, "\t %d [fillcolor=black]; \n", graph->start[BLACK]);

  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    fprintf(fptr, "\t %d [fillcolor=red]; \n", graph->objectives[i]);
  }
}

// TODO: voir pb de coverage
void graph_show(struct graph_t* graph, char* file_name)
{
  FILE* fptr = fopen(file_name, "w");

  fprintf(fptr, "graph G { \n");

  graph_style(graph, fptr);

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
      fprintf(fptr, "}; \n");
  }

  fprintf(fptr, "}");
  fclose(fptr);
}
