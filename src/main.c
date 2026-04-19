#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h"
#include "graph_impl.h"
#include "move.h"
#include "player.h"

typedef struct ServerPlayer {
    void* handle;
    char const* (*get_player_name)(void);
    void (*initialize)(unsigned int, struct graph_t*);
    struct move_t (*play)(const struct move_t previous_moves[NUM_PLAYERS]);
    void (*finalize)(void);
};

ServerPlayer load_player(const char* library_path) {
    ServerPlayer sp;
    sp.handle = dlopen(library_path, RTLD_LAZY);
    if (!sp.handle) {
        fprintf(stderr, "Erreur de chargement de %s: %s\n", library_path, dlerror());
        exit(EXIT_FAILURE);
    }
    sp.get_player_name = dlsym(sp.handle, "get_player_name");
    sp.initialize      = dlsym(sp.handle, "initialize");
    sp.play            = dlsym(sp.handle, "play");
    sp.finalize        = dlsym(sp.handle, "finalize");

    return sp;
}

unsigned int random_or_config() {
  return 0;
}

int main(int argc, char* argv[]) {
  // --- Game variable ---

  int m = 6;
  int M = -1;
  enum graph_type_t type = TRIANGULAR;
  
  // --- Command line options ---

  int opt;
  while ((opt = getopt(argc, argv, "m:M:t:")) != -1)
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
        type = graph_type(optarg[0], optarg[1]);
        break;
    }
    ServerPlayer players[NUM_PLAYERS];
    players[0] = load_player("./libclient1.so");
    players[1] = load_player("./libclient2.so");

    //random_or_config() is a fonction determine which player will start first randomly 
    unsigned int current_player_id = random_or_config(); 
    struct graph_t graph = graph_init(m, type);
    struct move_t previous_moves[NUM_PLAYERS] = {{.t = NO_TYPE}, {.t = NO_TYPE}};
    enum player_color_t players_color[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
      players_color[i] = (i == current_player_id) ? BLACK : WHITE;
      players[i].initialize(i, &graph);
      printf("The player %d (%s) join the party", i, players[i].get_player_name());
    }
    bool game_over = false;
    while (!game_over) {
      printf("--- Tour du joueur %d ---\n", current_p);
      struct move_t move = players[current_p].play(previous_moves);
      




    }

  }

  
  
  return EXIT_SUCCESS;
}
