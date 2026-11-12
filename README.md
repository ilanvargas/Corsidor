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
│   ├── graph.h / graph_impl.c/h   # Graph construction and manipulation (GSL sparse matrices)
│   ├── move.h / move_check.h      # Move type definitions and validation interface
│   ├── move_valid.c               # Move validation logic (MOVE, JUMP, WALL)
│   ├── waterloo.c/h               # Pathfinding / game strategy
│   ├── player.h                   # Common client/player interface
│   ├── player_scarecrow.c         # AI client: Scarecrow
│   ├── player_tourist.c/h         # AI client: Tourist player interface
│   ├── tourist.c                  # AI client: Tourist strategy implementation
│   └── main.c                     # Game server entry point
├── test/
│   ├── test_graph_impl.c   # Unit tests for the graph engine
│   ├── test_move_valid.c   # Unit tests for move validation
│   └── test_waterloo.c     # Unit tests for pathfinding
├── install/                # Build output: compiled binaries and shared libraries (generated, not versioned)
└── Makefile
```

---

## Dependencies

- **GCC** with C99 support
- **GSL** (GNU Scientific Library) — used for sparse matrix representation (`gsl_spmatrix_uint`)
- **Graphviz** (`dot`) — optional, only needed for graph visualization (`make dot_test`)

These libraries are **not** vendored in this repository.
Each developer must install them locally, either via a system package manager or by
building them from source, and point the `Makefile` at the install location.

### Option A — Install via package manager (recommended)

Debian / Ubuntu:

```bash
sudo apt install libgsl-dev graphviz
```

macOS (Homebrew):

```bash
brew install gsl graphviz
```

Then build with the install prefix used by your package manager (usually `/usr` on
Linux, `/opt/homebrew` or `/usr/local` on macOS):

```bash
make GSL_PATH=/usr DOT=dot
```

### Option B — Manual / local install (e.g. no sudo access, school machines)

Download and build the libraries yourself, then point the Makefile at the resulting
install directory:

- GSL sources: https://ftp.gnu.org/gnu/gsl/
- Graphviz sources/releases: https://graphviz.org/download/

```bash
make GSL_PATH=/path/to/gsl/install DOT=/path/to/graphviz/install/bin/dot
```

`GSL_PATH` must point to a prefix containing `include/` and `lib/` (or `lib64/`)
subdirectories for GSL. `DOT` must point directly at the `dot` executable.

> Note: `GSL_PATH` and `DOT` have no default value in the `Makefile`. You must pass
> them explicitly on every `make` invocation (or `export` them in your shell), for
> example by adding them to a local, untracked `Makefile.local` / shell profile.

---

## Build

```bash
# Build server + clients
make GSL_PATH=/usr DOT=dot

# Build and run tests
make GSL_PATH=/usr test

# Build, then copy binaries to install/
make GSL_PATH=/usr install

# Clean build artifacts
make clean
```

---

## Running the Server

```bash
./install/server
```

Clients are loaded at runtime as shared libraries via `dlopen`.

Options:
```
-m <number>      : size of one side of the graph (default: 5)
-M <number>      : maximum number of turns (default: infinite)
-t <graph_type>  : type of graph (default: parallelogram)
-g               : generates a .dot file representing the board
```

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

## AI Clients

| Client | Source | Description |
|--------|--------|--------------|
| `tourist.so` / `zzztourist.so` | `src/tourist.c`, `src/player_tourist.c/h` | Pathfinding-oriented strategy, tracks its own and the opponent's position/path |
| `zscarecrow.so` | `src/player_scarecrow.c` | Simpler heuristic-based strategy |

The set of clients built by `make client` is defined by the `CLIENTS` variable in the
`Makefile`.

---

## Tests

Unit tests are compiled as standalone executables and cover:

- **`test_graph_impl`** — graph construction and manipulation
- **`test_move_valid`** — move, jump, and wall validation; path direction angle checks
- **`test_waterloo`** — pathfinding correctness

Run with Valgrind:

```bash
valgrind --leak-check=full ./install/test_move_valid
```

---

## Graph Visualization

Requires Graphviz (see [Dependencies](#dependencies)):

```bash
make GSL_PATH=/usr DOT=dot dot_test
```

Generates `.dot` files and renders them as PDFs using Graphviz. Output files:
`test1.pdf`, `parallelogram_m5_test.pdf`, `parallelogram_m9_test.pdf`, `triangular_test.pdf`.

---

## Coverage

Build flags include `--coverage`. After running tests, generate a coverage report with:

```bash
gcov src/*.c
```
