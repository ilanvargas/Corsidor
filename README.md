# Corsidor — S6 Project

**VARGAS Ilan, HSSASSA Mohsine, DASSOULI Youssef**  
ENSEIRB-MATMECA — Bordeaux INP

---

## Overview

Corsidor is a two-player hexagonal board game inspired by Quoridor, implemented in C. 
Players move on a hexagonal grid and can place walls to block their opponent's path. 
The project includes a game server, a graph engine, clients loaded dynamically as shared 
libraries, and a suite of unit tests.

---

## Project Structure

```
corsidor-S6/
├── src/
│   ├── graph_impl.c/h      # Graph construction and manipulation (GSL sparse matrices)
│   ├── move_valid.c/h      # Move validation logic (MOVE, JUMP, WALL)
│   ├── waterloo.c/h        # Pathfinding / game strategy
│   ├── player_Scarecrow.c  # AI client implementation
│   └── main.c              # Game server entry point
├── test/
│   ├── test_move_valid.c   # Unit tests for move validation
│   └── test_waterloo.c     # Unit tests for pathfinding
├── install/                # Compiled binaries and shared libraries
├── gsl/                    # Local GSL installation (if applicable)
└── Makefile
```

---

## Dependencies

- **GCC** with C99 support
- **GSL 2.6** (GNU Scientific Library) — used for sparse matrix representation (`gsl_spmatrix_uint`)
- **Graphviz** (`dot`) — optional, for graph visualization

The GSL path is by default customized to school machine

1. `/net/ens/renault/save/gsl-2.6/install` (school machines)

To use a custom GSL path:

```bash
make GSL_PATH=/path/to/gsl
```

---

## Build

```bash
# Build server + clients
make

# Build and run tests
make test

# Build, then copy binaries to install/
make install

# Clean build artifacts
make clean
```

---

## Running the Server

```bash
./intall/server
```

clients are loaded at runtime as shared libraries via `dlopen`. 

Options:
    -m <number> : size of one side of the graph (default: 5)
    -M <number> : maximum number of turns (default: infinite)
    -t <graph_type> : type of graph (default: parallelogram)
    -g : generates a .dot file representing the board

---

## Graph Representation

The board is modeled as a directed graph using a **CSR sparse matrix** (`gsl_spmatrix_uint`). Each non-zero entry `t[i][j]` encodes the direction from vertex `i` to vertex `j` (E, W, NE, NW, SE, SW). Walls are encoded by setting a bitmask (`WALL_MASK`) on the corresponding edge values.

Two board shapes are supported:

| Type | Description |
|------|-------------|
| `PARALLELOGRAM` | Standard rectangular hexagonal grid of size m×m |
| `TRIANGULAR` | Triangular hexagonal grid |

---

## Move Types

| Type | Description |
|------|-------------|
| `MOVE` | Standard displacement along edges (up to 3 steps straight, or 2 at 60°) |
| `JUMP` | Jump over the opponent to an adjacent free vertex |
| `WALL` | Place a wall across two consecutive edges sharing the same origin vertex |

---

## Tests

Unit tests are compiled as standalone executables and cover:

- **`test_move_valid`** — move, jump, and wall validation; path direction angle checks
- **`test_waterloo`** — pathfinding correctness

Run with Valgrind:

```bash
valgrind --leak-check=full ./install/test_move_valid
```

---

## Graph Visualization

```bash
make dot_test
```

Generates `.dot` files and renders them as PDFs using Graphviz. Output files: `test1.pdf`, `parallelogram_test.pdf`, `triangular_test.pdf`.

---

## Coverage

Build flags include `--coverage`. After running tests, generate a coverage report with:

```bash
gcov src/*.c
```
