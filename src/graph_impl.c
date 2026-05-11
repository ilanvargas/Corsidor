#include <stdio.h>
#include <stdlib.h>

#include "graph_impl.h"

#define UNUSED(x) (void)(x)

// --- Initialialiser graph variables ---
// RETURN: Graph side size

size_t f_init_parallelogram(size_t m, struct graph_t* graph) {
  graph->type = PARALLELOGRAM;

  graph->num_vertices = m * m;
  graph->num_edges = 3U * graph->num_vertices - 4U * m + 1;

  graph->start[BLACK] = m * (m / 2);
  graph->start[WHITE] = m * (m / 2) + m - 1;

  graph->num_objectives = 4;
  vertex_t* objectives = malloc(4 * sizeof(vertex_t));
  objectives[0] = 0;
  objectives[1] = m - 1;
  objectives[2] = m * (m - 1);
  objectives[3] = m * m - 1;
  graph->objectives = objectives;

  return m;
}

size_t f_init_triangular(size_t m, struct graph_t* graph) {
  graph->type = TRIANGULAR;

  size_t n = 2 * m - 1;

  graph->num_vertices = 3 * m * m - 3 * m + 1;
  graph->num_edges = 9 * m * m - 15 * m + 6;

  graph->start[BLACK] = n * (n / 2);
  graph->start[WHITE] = n * (n / 2) + n - 1;

  graph->num_objectives = 4;
  vertex_t* objectives = malloc(4 * sizeof(vertex_t));
  objectives[0] = (m / 2) * n + 1;
  objectives[1] = (m / 2) * n + m;
  objectives[2] = n * (n - m / 2) - m - 1;
  objectives[3] = n * (n - m / 2) - 2;
  graph->objectives = objectives;

  return n;
}

// --- Conditions for graph construction ---

int f_cond_parallelogram(size_t m, size_t i, size_t j) {
  UNUSED(m);
  UNUSED(i);
  UNUSED(j);
  return 1;
}

int f_cond_triangular(size_t m, size_t i, size_t j) {
  return (j < m + i) && (i < m + j);
}

// --- Graph init function ---

struct graph_t* graph_init(size_t m, enum graph_type_t type) {
  struct graph_t* graph = malloc(sizeof(struct graph_t));

  int (*f_cond)(size_t m, size_t i, size_t j);
  size_t n;
  switch (type) {
    case PARALLELOGRAM:
      n = f_init_parallelogram(m, graph);
      f_cond = f_cond_parallelogram;
      break;
    case TRIANGULAR:
      n = f_init_triangular(m, graph);
      f_cond = f_cond_triangular;
      break;
    default:
      return NULL;
  }

  gsl_spmatrix_uint* t = gsl_spmatrix_uint_alloc(n * n, n * n);
  int dir_i[] = {0, -1, -1, 0, 1, 1, 0};
  int dir_j[] = {0, 0, -1, -1, 0, 1, 1};

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      for (enum dir_t d = FIRST_DIR; d <= LAST_DIR; d++) {
        int ii = (int)i + dir_i[d];
        int jj = (int)j + dir_j[d];
        if (ii >= 0 && (size_t)ii < n && jj >= 0 && (size_t)jj < n) {
          if (f_cond(m, i, j) && f_cond(m, (size_t)ii, (size_t)jj))
            gsl_spmatrix_uint_set(
              t, i * n + j, (size_t)ii * n + (size_t)jj, d);
        }
      }
    }
  }

  graph->t = gsl_spmatrix_uint_crs(t);
  free(t);
  return graph;
}

// --- Graph functions ---

void graph_free(struct graph_t* graph) {
  gsl_spmatrix_uint_free(graph->t);
  free(graph->objectives);
  free(graph);
}

/*
 * Translate a direction vector (di, dj) to a direction enum.
 */
enum dir_t translate_dir(int di, int dj) {
  if (di < 0 && dj == 0)
    return NW;
  else if (di < 0 && dj < 0)
    return NE;
  else if (di == 0 && dj < 0)
    return E;
  else if (di > 0 && dj == 0)
    return SE;
  else if (di > 0 && dj > 0)
    return SW;
  else if (di == 0 && dj > 0)
    return W;
  else
    return NO_EDGE;
}

unsigned int* get_edge(gsl_spmatrix_uint* csr, vertex_t fr, vertex_t to) {
    for (int k = csr->p[fr]; k < csr->p[fr + 1]; k++) {
        if ((vertex_t)csr->i[k] == to)
            return &csr->data[k];
    }
    return NULL;
}

void build_wall(struct graph_t* graph, struct edge_t e[2]) {
  // Wall needs to be read on both sides
  gsl_spmatrix_uint* csr = graph->t;

  unsigned int* val0_fr = get_edge(csr, e[0].fr, e[0].to);
  unsigned int* val0_to = get_edge(csr, e[0].to, e[0].fr);

  unsigned int* val1_fr = get_edge(csr, e[1].fr, e[1].to);
  unsigned int* val1_to = get_edge(csr, e[1].to, e[1].fr);

  *val0_to |= WALL_MASK;
  *val0_fr |= WALL_MASK;
  *val1_to |= WALL_MASK;
  *val1_fr |= WALL_MASK;
}

// --- Functions for graph representation ---

void graph_show_style(struct graph_t* graph, FILE* fptr) {
  fprintf(fptr,
          "\t node [shape=circle, style=filled,fixedsize=true, width=0.5]; \n");

  fprintf(fptr, "\t %d [fillcolor=white]; \n", graph->start[WHITE]);
  fprintf(fptr, "\t %d [fillcolor=black]; \n", graph->start[BLACK]);

  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    fprintf(fptr, "\t %d [fillcolor=red]; \n", graph->objectives[i]);
  }
}

void graph_show(struct graph_t* graph, char* file_name) {
  FILE* fptr = fopen(file_name, "w");

  fprintf(fptr, "graph G {\n");

  graph_show_style(graph, fptr);

  gsl_spmatrix_uint* csr = graph->t;
  for (size_t i = 0; i < csr->size1; i++) {
    for (int k = csr->p[i]; k < csr->p[i + 1]; k++) {
      size_t j = csr->i[k];

      if (j >= i) {
        if (IS_WALL(csr->data[k]))
          fprintf(fptr, "\"%zu\" -- \"%zu\" [color=red];\n", i, j);
        else
          fprintf(fptr, "\"%zu\" -- \"%zu\";\n", i, j);
      }
    }
  }

  fprintf(fptr, "}\n");
  fclose(fptr);
}
