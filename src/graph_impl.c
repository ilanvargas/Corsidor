#include <stdio.h>

#include "graph_impl.h"

#define UNUSED(x) (void)(x)

// --- Initialialiser graph variables ---
// RETURN: Square matrix size

size_t f_init_parallelogram(int m, struct graph_t* graph)
{
  graph->type = PARALLELOGRAM;

  graph->num_vertices = m*m;
  graph->num_edges = 3*graph->num_vertices - 4*m + 1;
  graph->t = gsl_spmatrix_uint_alloc(graph->num_vertices, graph->num_vertices);

  graph->start[BLACK] = m * (m - 1) / 2 + 1;
  graph->start[WHITE] = m * (m + 1) / 2;

  graph->num_objectives = 4;
  vertex_t* objectives = malloc(4*sizeof(vertex_t));
  objectives[0] = 0;
  objectives[1] = m-1;
  objectives[2] = m*(m - 1);
  objectives[3] = m*m - 1;
  graph->objectives = objectives;

  return (size_t)m;
}

// --- Conditions for graph construction ---

int f_cond_parallelogram(int m, size_t i, size_t j)
{
  UNUSED(m); UNUSED(i); UNUSED(j);
  return 1;
}

// --- Graph init function ---

struct graph_t graph_init(int m, enum graph_type_t type)
{
  struct graph_t graph;

  size_t (*f_init)(int m, struct graph_t*);
  int (*f_cond)(int m, size_t i, size_t j);

  switch (type) {
    case PARALLELOGRAM:
      f_init = f_init_parallelogram;
      f_cond = f_cond_parallelogram;
      break;
    default:
      f_init = f_init_parallelogram;
      f_cond = f_cond_parallelogram;
      break;
  }

  size_t n = f_init(m, &graph);

  int dir_i[] = {1, 1, 0, -1, -1, 0};
  int dir_j[] = {-1, 0, 1, 1, 0, -1};

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      for (enum dir_t d = FIRST_DIR; d < LAST_DIR; d++) {

        int ii = (int)i + dir_i[d];
        int jj = (int)j + dir_j[d];
        printf("d %d,i %ld %ld,ii %d %d,m %ld %ld \n", d, i, j, ii, jj, i*n+j, (size_t)ii*n+(size_t)jj);
        if (ii >= 0 && ii < (int)n && jj >= 0 && jj < (int)n) {
          printf("yes\n");
          gsl_spmatrix_uint_set(graph.t, i*n+j, (size_t)ii*n+(size_t)jj, 
              (f_cond(m, i, j) && f_cond(m, (size_t)ii, (size_t)jj)) ? d : 0);
        }
      }
    }
  }

  return graph;
}

// --- Functions for graph representation ---

void graph_show_style(struct graph_t* graph, FILE* fptr)
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

  graph_show_style(graph, fptr);

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
