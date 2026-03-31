#include <getopt.h>
#include <stdlib.h>

#include "graph.h"

int main(int argc, char* argv[]) 
{
  int m = 6;
  int M = -1;
  enum graph_type_t game_graph_type = TRIANGULAR;
  
  // --- Command line options

  int opt;
  while ((opt = getopt(argc, argv, "s:t:n:q:igb")) != -1)
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

  
  
  return EXIT_SUCCESS;
}
