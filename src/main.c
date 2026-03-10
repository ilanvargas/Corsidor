#include <getopt.h>
#include <stdlib.h>

#include "graph.h"

int main(int argc, char* argv[]) 
{
  int m = 6;
  enum graph_type_t graph_type = TRIANGULAR;
  int M = -1;
  
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

        switch (optarg[0]) 
        {
          case 'P':
            graph_type = PARALLELOGRAM;
            break;
          case 'C':
            graph_type = CYCLIC;
            break;
          case 'H':
            graph_type = HOLEY;
            break;
          case 'L':
            graph_type = LINEAR;
            break;
          case 't':
            graph_type = TRIANGULAR_RANDOM;
            break;
          case 'h':
            graph_type = HOLEY_RANDOM;
            break;
          case 'S':
            graph_type = SPAN;
            break;
        }
        break;
    }
  }
  
  return EXIT_SUCCESS;
}
