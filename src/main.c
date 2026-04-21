#include <dlfcn.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "graph_impl.h"
#include "move.h"
#include "move_valid.h"
#include "player.h"

// Function pointers for dynamic loading
typedef char const* (*get_player_name_t)();
typedef void (*initialize_t)(unsigned int, struct graph_t*);
typedef struct move_t (*play_t)(const struct move_t[]);
typedef void (*finalize_t)();

typedef struct ServerPlayer {
  void* handle;
  char const* (*get_player_name)(void);
  void (*initialize)(unsigned int, struct graph_t*);
  struct move_t (*play)(const struct move_t previous_moves[NUM_PLAYERS]);
  void (*finalize)(void);
};

struct ServerPlayer load_player(const char* library_path) {
  struct ServerPlayer sp;
  sp.handle = dlopen(library_path, RTLD_LAZY);
  if (!sp.handle) {
    fprintf(stderr, "Erreur de chargement de %s: %s\n", library_path,
            dlerror());
    exit(EXIT_FAILURE);
  }
  sp.get_player_name = dlsym(sp.handle, "get_player_name");
  sp.initialize = dlsym(sp.handle, "initialize");
  sp.play = dlsym(sp.handle, "play");
  sp.finalize = dlsym(sp.handle, "finalize");

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
  while ((opt = getopt(argc, argv, "m:M:t:")) != -1) {
    switch (opt) {
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
  }
  struct ServerPlayer players[NUM_PLAYERS];
  players[0] = load_player("./libclient1.so");
  players[1] = load_player("./libclient2.so");

  // random_or_config() is a fonction determine which player will start first
  // randomly
  unsigned int current_player = random_or_config();
  // Initialize graph
  struct graph_t* graph = malloc(sizeof(struct graph_t));
  *graph = graph_init(m, type);
  // Player positions
  vertex_t positions[NUM_PLAYERS] = {graph->start[0], graph->start[1]};
  // Visited objectives
  int visited1[graph->num_objectives];
  int visited2[graph->num_objectives];
  memset(visited1, 0, sizeof(visited1));
  memset(visited2, 0, sizeof(visited2));

  struct move_t previous_moves[NUM_PLAYERS] = {{.t = NO_TYPE}, {.t = NO_TYPE}};
  enum player_color_t players_color[NUM_PLAYERS];
  for (unsigned int i = 0; i < NUM_PLAYERS; i++) {
    players_color[i] = (i == current_player) ? BLACK : WHITE;
    players[i].initialize(i, graph);
    printf("The player %d (%s) join the party", i,
           players[i].get_player_name());
  }

  int winner = -1;

  while (winner == -1) {
    struct move_t move;
    if (current_player == 0) {
      move = players[0].play(previous_moves);
    } else {
      move = players[1].play(previous_moves);
    }

    // Validate move and check for Waterloo
    if (!move_is_valid(graph, move, previous_moves, positions)) {
      fprintf(stderr, "Invalid move by player %d\n", current_player);
      // End game
      winner = 1 - current_player;  // Opponent wins
      printf("Player %d wins by invalid move!\n", winner);
      break;
    }
    if (move_is_waterloo(graph, move, positions)) {
      printf("Player %d played a Waterloo move!\n", current_player);
      // Handle Waterloo move
      move = waterloo_move(move, graph, positions);
      previous_moves[current_player] = move;
      continue;  // Skip turn
    }

    // Apply the move to the graph
    if (move.t == MOVE) {
      positions[move.c] = move.m;
      printf("Player %d moved to position %u\n", current_player, move.m);
    } else if (move.t == WALL) {
      // Remove the edges for the wall
      gsl_spmatrix_uint_set(graph->t, move.e[0].fr, move.e[0].to, NO_EDGE);
      gsl_spmatrix_uint_set(graph->t, move.e[1].fr, move.e[1].to, NO_EDGE);
      // Also remove reverse edges if undirected
      gsl_spmatrix_uint_set(graph->t, move.e[0].to, move.e[0].fr, NO_EDGE);
      gsl_spmatrix_uint_set(graph->t, move.e[1].to, move.e[1].fr, NO_EDGE);
      printf("Player %d placed a wall\n", current_player);
    } else if (move.t == WATERLOO) {
      // For WATERLOO, position may have changed
      if (move.m != positions[move.c]) {
        positions[move.c] = move.m;
        printf("Player %d stopped at position %u due to wall\n", current_player,
               move.m);
      }
    }

    // Mark visited objectives (check all objectives)
    for (unsigned int i = 0; i < graph->num_objectives; i++) {
      if (positions[current_player] == graph->objectives[i]) {
        if (current_player == 0) {
          visited1[i] = 1;
        } else {
          visited2[i] = 1;
        }
      }
    }

    // Update previous moves
    previous_moves[current_player] = move;

    // Check for win condition
    // Player wins by visiting all objectives and returning to start
    int* visited = (current_player == 0) ? visited1 : visited2;
    int all_objectives_visited = 1;
    for (unsigned int i = 0; i < graph->num_objectives; i++) {
      if (!visited[i]) {
        all_objectives_visited = 0;
        break;
      }
    }
    if (all_objectives_visited &&
        positions[current_player] == graph->start[current_player]) {
      winner = current_player;
      printf("Player %d wins by visiting all objectives and returning home!\n",
             winner);
    }
    current_player = 1 - current_player;
  }

  // Finalize
  for (int i = 0; i < NUM_PLAYERS; i++) {
    players[i].finalize();
    dlclose(players[i].handle);
  }

  // Free graph
  free(graph->objectives);
  gsl_spmatrix_uint_free(graph->t);
  free(graph);

  return EXIT_SUCCESS;
}
