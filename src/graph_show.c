#include <stdio.h>

#define FILE_DOT(file) file_name ".dot"

void graph_show(struct graph_t* graph, char* file_name)
{
  FILE* fptr = fopen(FILE_DOT(file_name), "w");

  fprintf(fptr, "graph G {\n");

  size_t N = graph->t->size1;
  for (int i = 0; i < N; i++) {
    
    int is_show = 0;
    for (int j = i; j < N; j++) {
      if (graph->t[i][j] > 0){
        if (!is_show) {
          is_show = 1;
          fprintf(fptr, "\t \"%d\" -- {", i);
        }
        fprintf(fptr, "\"%d\" ", j);
      }
      if (is_show)
        fprintf(fptr, "}\\n");
    }
  }
  fclose(fptr);
}
