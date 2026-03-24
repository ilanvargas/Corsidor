#include <getopt.h>

#include "graph.h"

/*
unsigned int m = 6;
int M = -1;
enum graph_type_t game_graph_type = TRIANGULAR;
*  
// --- Command line options

int opt;
while ((opt = getopt(argc, argv, "m:M:t")) != -1)
  {
    switch (opt)
      {
      case 'm':
        m = atoi(optarg);
        break;
      case 'M':
        M = atoi(optarg);
        break;
      case 't':
        game_graph_type = graph_type(optarg[0], optarg[1]);
        break;
      }
  }

if ( game_graph_type == TRIANGULAR ){
  unsigned int num_vertices = 3 * m * m - 3 * m + 1;
  unsigned int num_edges = 9 * m * m - 15 * m + 6;
  vertex_t position_black = (m - 1) * m + ((m - 2) * (m - 1)) / 2 + 1;
  vertex_t position_white = m * m + (m * (m - 1)) / 2;
  gsl_spmatrix_uint *m = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
}
if ( game_graph_type == PARALLELOGRAM ){
  unsigned int num_vertices = m * m;
  unsigned int num_edges = 3 * num_vertices - 4 * m + 1;
  vertex_t position_black = m * (m - 1) / 2 + 1;
  vertex_t position_white = m * (m + 1) / 2;
  gsl_spmatrix_uint *matrix = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
  for(int i = 0;i < m; i++){
    for(int j = m*i;j < m*(i + 1); j++){
      if ((j + 1) % m != 0){
        gsl_spmatrix_uint_set(matrix,j,j+1,3);
      }
      if(j % m != 0){
        gsl_spmatrix_uint_set(matrix,j+1,j,6);
      }
      if (i != m - 1){
        gsl_spmatrix_uint_set(matrix,j,j+m,5);
      }
      if (i != 0){
        gsl_spmatrix_uint_set(matrix,j+m,j,2);
      }
      if (((j+1) % m != 0) && (i != m - 1)){
        gsl_spmatrix_uint_set(matrix,j,j+m+1,4);
      }
      if ((j % m != 0) && (i != 0)){
        gsl_spmatrix_uint_set(matrix,j+m+1,j,1);
      }
    }
  }
}


gsl_spmatrix_uint *make_graph(){
  gsl_spmatrix_uint *matrix = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
  


}
*/

struct graph_t *generate_PARALLELOGRAM_graph(unsigned int m){
  struct graph_t *graph = malloc(sizeof(struct graph_t));
  graph->type = PARALLELOGRAM;
  graph->num_objectives = 4;
  graph->num_vertices = 3 * m * m - 3 * m + 1;
  graph->num_edges = 3 * graph->num_vertices - 4 * m + 1;
  vertex_t position_black = m * (m - 1) / 2 + 1;
  vertex_t position_white = m * (m + 1) / 2;
  graph->start[0] = position_black;
  graph->start[1] = position_white;
  vertex_t *objectives = malloc(4*sizeof(vertex_t));
  vertex_t obj1 = 0;
  vertex_t obj2 = m - 1;
  vertex_t obj3 = m * (m - 1);
  vertex_t obj4 = m * m - 1;
  objectives[0] = obj1;
  objectives[1] = obj2;
  objectives[2] = obj3;
  objectives[3] = obj4;
  graph->objectives = objectives;
  gsl_spmatrix_uint *matrix = gsl_spmatrix_uint_alloc(graph->num_vertices, graph->num_vertices);
  for(unsigned int i = 0;i < m; i++){
    for(unsigned int j = m*i;j < m*(i + 1); j++){
      if ((j + 1) % m != 0){
        gsl_spmatrix_uint_set(matrix,j,j+1,3);
      }
      if(j % m != 0){
        gsl_spmatrix_uint_set(matrix,j+1,j,6);
      }
      if (i != m - 1){
        gsl_spmatrix_uint_set(matrix,j,j+m,5);
      }
      if (i != 0){
        gsl_spmatrix_uint_set(matrix,j+m,j,2);
      }
      if (((j+1) % m != 0) && (i != m - 1)){
        gsl_spmatrix_uint_set(matrix,j,j+m+1,4);
      }
      if ((j % m != 0) && (i != 0)){
        gsl_spmatrix_uint_set(matrix,j+m+1,j,1);
      }
    }
  }
  graph->t = matrix;
  return graph;
}