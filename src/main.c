#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "move_check.h"
#include "player.h"

struct player_t {
  void* handle;
  char const* (*get_player_name)(void);
  void (*initialize)(unsigned int, struct graph_t*);
  struct move_t (*play)(const struct move_t previous_moves[NUM_PLAYERS]);
  void (*finalize)(void);
};

/** Loads player from dynamic librairy
 * Opens dlopen, remember to close it
 */
struct player_t load_player(const char* library_path) {
  struct player_t p;
  p.handle = dlopen(library_path, RTLD_LAZY);
  if (!p.handle) {
    fprintf(stderr, "Erreur de chargement de %s: %s\n", library_path,
            dlerror());
    exit(EXIT_FAILURE);
  }
  p.get_player_name = (char const* (*)(void))dlsym(p.handle, "get_player_name");
  p.initialize =
      (void (*)(unsigned int, struct graph_t*))dlsym(p.handle, "initialize");
  p.play = (struct move_t (*)(const struct move_t*))dlsym(p.handle, "play");
  p.finalize = (void (*)(void))dlsym(p.handle, "finalize");

  return p;
}

/** Verify is player reached an objective
 *  If its the case updated p_num_visited and p_is_visited
 *  and return 1
 */
int is_player_visit(struct graph_t* graph,
                    vertex_t pos,
                    unsigned int* num_visited,
                    int* is_visited) {
  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    if (pos == graph->objectives[i] && is_visited[i] == 0) {
      printf("An objective was reached\n");
      (*num_visited)++;
      is_visited[i] = 1;

      return 1;
    }
  }

  return 0;
}

/** Verify is game is finished and who won */
int is_end_game(struct graph_t* graph,
                int turn,
                int M,
                unsigned int p_num_visited[NUM_PLAYERS],
                vertex_t p_pos[NUM_PLAYERS]) {
  if (M > 0 && turn > M) {
    printf("Max turn reached, game is over\n");
    return 1;
  }

  // if a player visited all objectives
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (p_num_visited[i] >= graph->num_objectives &&
        p_pos[i] == graph->start[i]) {
      printf(
          "%s visited all objectives and won game and came back. "
          "Congratulations!\n",
          (i == 0) ? "BLACK" : "WHITE");
      return 1;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
  // --- Game variable ---

  int m = 5;
  int M = -1;
  enum graph_type_t type = PARALLELOGRAM;
  int is_dot_gen = 0;

  // --- Command line options ---

  int opt;
  while ((opt = getopt(argc, argv, "m:M:t:g")) != -1) {
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
      case 'g':
        is_dot_gen = 1;
        break;
    }
  }

  // --- Initialisation ---

  // pick a random color based on time
  srand(time(NULL));
  int color = rand() % NUM_PLAYERS;

  struct player_t players[NUM_PLAYERS];
  players[color] = load_player("./scarecrow.so");
  players[1 - color] = load_player("./foenem.so");

  struct graph_t* graph = graph_init(m, type);

  int p_walls[NUM_PLAYERS] = {graph->num_edges / 16, graph->num_edges / 16};
  vertex_t p_pos[NUM_PLAYERS] = {graph->start[BLACK], graph->start[WHITE]};
  vertex_t p_prev_pos[NUM_PLAYERS] = {UINT_MAX, UINT_MAX};
  struct move_t p_prev_moves[NUM_PLAYERS] = {{.t = NO_TYPE}, {.t = NO_TYPE}};

  unsigned int p_num_visited[NUM_PLAYERS] = {0};
  int p_is_visited[NUM_PLAYERS][graph->num_objectives];
  memset(p_is_visited, 0, sizeof(p_is_visited));

  for (int i = 0; i < NUM_PLAYERS; i++) {
    players[i].initialize(i, graph_init(m, type));
    printf("The player %s join the game as %s\n", players[i].get_player_name(),
           i == 0 ? "BLACK" : "WHITE");
  }

  // Creation of dot file for board represantation
  // If asked
  if (is_dot_gen) {
    printf("\e[1mdot file created: game.dot\e[0m\n");
    graph_show(graph, "game.dot");
  }

  // --- Game ---

  int turn = 0;
  while (!is_end_game(graph, turn, M, p_num_visited, p_pos)) {
    if (M > 0)
      printf("======= turn %d out of %d =======\n", turn, M);
    else
      printf("============ turn %d ============\n", turn);
    printf("\t BLACK position: %u, objectives captured: %d\n", p_pos[BLACK],
           p_num_visited[BLACK]);
    printf("\t WHITE position: %u, objectives captured: %d\n", p_pos[WHITE],
           p_num_visited[WHITE]);
    printf("=================================\n");

    struct move_t move;
    int color_turn = turn % NUM_PLAYERS;
    char* color_str = (color_turn == 0) ? "BLACK" : "WHITE";
    char* color_op_str = (color_turn == 0) ? "WHITE" : "BLACK";

    if (color_turn == BLACK)
      move = players[BLACK].play(p_prev_moves);
    else
      move = players[WHITE].play(p_prev_moves);

    if (!move_is_valid(graph, &move, p_pos, p_prev_pos, p_walls)) {
      fprintf(stderr, "Invalid move by %s\n", color_str);
      printf("%s plays {.t= %d, .m= %d ...} -> invalid\n", color_str, move.t,
             move.m);
      printf("%s wins by invalid move!\n", color_op_str);  // Opponent wins
      break;
    }

    // move application
    switch (move.t) {
      case MOVE:
        vertex_t tmp_movem = move.m;
        if (convert_waterloo(graph, &move, p_pos))
          printf(
              "%s tried to MOVE to %d but made waterloo move. Player moves to "
              "%d\n",
              color_str, tmp_movem, move.m);
        else
          printf("%s moves to %d\n", color_str, move.m);

        p_prev_pos[color_turn] = p_pos[color_turn];
        p_pos[color_turn] = move.m;
        is_player_visit(graph, p_pos[color_turn], &p_num_visited[color_turn],
                        p_is_visited[color_turn]);

        break;
      case JUMP:
        vertex_t tmp_jumpm = move.m;
        if (convert_waterloo(graph, &move, p_pos))
          printf(
              "%s tried to JUMP to %d but made waterloo move. Player: stays in "
              "place\n",
              color_str, tmp_jumpm);
        else {
          printf("%s JUMPS to %d\n", color_str, move.m);

          p_prev_pos[color_turn] = p_pos[color_turn];
          p_pos[color_turn] = move.m;
        }

        break;
      case WALL:
        struct edge_t tmp_e0 = move.e[0];
        struct edge_t tmp_e1 = move.e[1];
        if (convert_waterloo(graph, &move, p_pos))
          printf(
              "%s tried to put WALL on edges (%d, %d) and (%d, %d) but made "
              "waterloo move. No wall is placed\n",
              color_str, tmp_e0.fr, tmp_e0.to, tmp_e1.fr, tmp_e1.to);
        else {
          printf("%s put WALL on edges (%d, %d) and (%d, %d)\n", color_str,
                 move.e[0].fr, move.e[0].to, move.e[1].fr, move.e[1].to);

          build_wall(graph, move.e);
        }

        break;
      default:
        exit(EXIT_FAILURE);  // Unreacheacle case
    }

    p_prev_moves[color_turn] = move;

    turn++;
  }

  // --- Finalize ---

  for (int i = 0; i < NUM_PLAYERS; i++) {
    players[i].finalize();
    dlclose(players[i].handle);
  }

  graph_free(graph);

  return EXIT_SUCCESS;
}
