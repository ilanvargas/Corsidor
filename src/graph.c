#include <getopt.h>

#include "graph.h"


unsigned int m = 6;
int M = -1;
enum graph_type_t game_graph_type = TRIANGULAR;
    
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
  unsigned int num_edges = 3 * num_vertices + 2;
  vertex_t position_black = m * (m - 1) / 2 + 1;
  vertex_t position_white = m * (m + 1) / 2;
  gsl_spmatrix_uint *m = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);

}


gsl_spmatrix_uint *make_graph(){
  gsl_spmatrix_uint *m = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
  


}
