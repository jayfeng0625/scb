# scb

A zero-malloc C23 chess game evaluator.

## Decision Drivers

- **C23 / stdbit.h** -- modern C with portable bit intrinsics, fallback to builtins
- **Bitboards** -- 64-bit integers for piece sets; fast attack generation
- **Stateless CLI** -- every call takes a FEN and returns a FEN; no server, no sessions
- **FEN + SAN** -- standard chess formats for position and move notation
- **Zero allocation** -- all buffers on the stack, no malloc
- **Pseudo-legal + filter** -- generate candidate moves, then discard those leaving king in check
- **Layered library** -- bitboard < position < movegen < notation < rules < render
- **Simplicity** -- clear names, short functions, few comments

## Build

```sh
make          # build the CLI binary at build/chess
make test     # build and run the test suite
make clean    # remove all build artifacts
```

Requires a C23-capable compiler (gcc 14+, clang 18+, Apple clang 16+).

## Quick Examples

### Start a new game

```sh
$ ./build/chess new-game
fen: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
status: normal
```

### Validate and apply a move

```sh
$ ./build/chess validate \
    --fen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" \
    --move "e4"
fen: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
status: normal
move: e4
```

### List legal moves

```sh
$ ./build/chess legal-moves \
    --fen "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
moves: a3 a4 b3 b4 c3 c4 d3 d4 e3 e4 f3 f4 g3 g4 h3 h4 Na3 Nc3 Nf3 Nh3
count: 20
```

### Render the board

```sh
$ ./build/chess render \
    --fen "r3k2r/pp3ppp/2n5/3p4/4P3/5N2/PPP2PPP/R3K2R w KQkq - 2 12"
fen: r3k2r/pp3ppp/2n5/3p4/4P3/5N2/PPP2PPP/R3K2R w KQkq - 2 12
status: normal
```

## Architecture

```
include/chess.h     Public API -- types, constants, function declarations
src/bitboard.c      Attack table init, sliding piece ray attacks
src/position.c      FEN parsing/serialization, make_move, piece_at/color_at
src/movegen.c       is_square_attacked, generate_legal_moves
src/notation.c      SAN parsing (parse_san) and formatting (format_san)
src/rules.c         Game status detection (checkmate, stalemate, draws)
src/render.c        Board rendering, status_name
cli/main.c          CLI entry point with four subcommands
tests/              Test suite (test.h harness, per-module test files)
```
