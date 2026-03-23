# scb — Design Specification

A stateless, pure C chess game evaluator built on bitboards. Exposes a CLI that takes FEN + SAN in, produces FEN + rendered board out. No built-in AI, no search, no history — scb does one thing well and leaves the rest to its consumers.

## Decision Drivers

These choices were made during brainstorming and shape every aspect of the design.

**C23 standard** — `<stdbit.h>` gives us first-class `popcount`/`ctz` for bitboards. `constexpr`, binary literals, and digit separators make board constants readable. GCC 15+ and Clang 18+ have full support; macOS ships Clang.

**Bitboard representation** — 12 bitboards (`pieces[2][6]`) indexed by color and piece type. Compact, composable with bitwise ops, O(1) access to any piece set. Redundant `occupied[2]` and `all` boards kept in sync for speed.

**Stateless CLI architecture** — scb is a pure function: FEN in, FEN out. It validates a single move against a single position, returns the result, and forgets. Game history, turn order, session state, and intelligence are the caller's responsibility. No session, no cleanup, no global state.

**FEN + SAN as interchange** — FEN is the universal standard for board state. SAN is the universal standard for moves. Every chess tool speaks both. Human-readable and agent-parseable.

**No malloc** — chess is a finite, bounded problem with a very large but limited state space; while practical limits exist on moves, positions, and notation length, exact maximums depend on definitions and edge cases. The key insight is that every buffer has a conservative known ceiling. The library calls zero `malloc` — all buffers are caller-owned stack arrays. No leak paths, no ownership questions.

**Pseudo-legal + filter for move generation** — generate all pseudo-legal moves, then filter by making each move and checking if the king is attacked. Simpler than direct legal generation, easy to verify correct, plenty fast for a stateless CLI.

**Layered library + CLI frontend** — `libchess` is a static library with a clean public API. The CLI is a thin driver. This allows a REPL, MCP tool, or any other frontend to be built on top later without restructuring.

**Simplicity** — prefer clear names and short functions over comments and boilerplate.

## Scope

scb is a **game-state evaluator**, not a chess engine. The distinction matters:

- **What scb does:** validate moves, generate legal moves, detect check/checkmate/stalemate/draws, parse and format positions. Given a FEN and a move, it answers "is this legal?" and "what's the resulting position?" with certainty.
- **What scb does not do:** evaluate position quality (material balance, king safety, piece activity), search for best moves (minimax, alpha-beta), or track game history (move logs, threefold repetition, PGN).

This is deliberate. scb is a focused, single-purpose tool — it owns the rules of chess and nothing else. Consumers bring their own:

| Concern | Consumer's responsibility | Examples |
|---------|--------------------------|----------|
| **Intelligence** | Search, evaluation, heuristics | Engine wrapper, neural net, LLM agent |
| **History** | Move logs, position tracking, threefold repetition | Database, in-memory list, session store |
| **Presentation** | SAN formatting, PGN export, GUI rendering | Chat UI, web frontend, terminal app |
| **Orchestration** | Turn order, time control, player management | Game server, MCP tool, CLI harness |

This separation means scb never needs to change when consumers evolve. A new evaluation function, a different storage backend, or an entirely new UI all compose with the same `libchess` API.

## Architecture

```
include/
  chess.h          — single public header

src/
  bitboard.c       — bit manipulation, square/coordinate conversion
  position.c       — FEN parse/serialize, init, make_move, piece_at
  movegen.c        — legal move generation, attack tables, is_square_attacked
  notation.c       — SAN parse/format, move disambiguation
  rules.c          — check, checkmate, stalemate, draw detection
  render.c         — ASCII board rendering with info panel

cli/
  main.c           — subcommand dispatch

Makefile
README.md
how-to-use.md
```

## Core Data Types

```c
typedef uint64_t Bitboard;
typedef int Square;  // 0 (a1) to 63 (h8)

enum { WHITE, BLACK };
enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum { MOVE_CASTLE_NONE, MOVE_CASTLE_KINGSIDE, MOVE_CASTLE_QUEENSIDE };

typedef struct {
    Bitboard pieces[2][6];
    Bitboard occupied[2];
    Bitboard all;
    int side;
    int castling;       // bitmask: WK=1, WQ=2, BK=4, BQ=8
    int ep_square;      // en passant target, or -1
    int halfmove;
    int fullmove;
} Position;

typedef struct {
    uint8_t from;
    uint8_t to;
    int piece;
    int captured;
    int promotion;      // piece type or -1
    int castling;       // MOVE_CASTLE_NONE, MOVE_CASTLE_KINGSIDE, MOVE_CASTLE_QUEENSIDE
    bool en_passant;
} Move;
```

`make_move` updates castling rights by checking `from`/`to` squares against king and rook home squares. This covers both moving your own rook/king and capturing the opponent's rook.

## Buffer Constants

```c
#define FEN_MAX      128
#define SAN_MAX      12
#define MOVES_MAX    256
#define RENDER_MAX   2048
```

## Public API

```c
// position
void position_init(Position *pos);
bool position_from_fen(Position *pos, const char *fen);  // validates: both kings present, no pawns on rank 1/8, side not-to-move not giving check
int  position_to_fen(const Position *pos, char *buf, int bufsize);

// moves — caller must pass array of at least MOVES_MAX elements
int  generate_legal_moves(const Position *pos, Move *moves);
bool make_move(Position *pos, const Move *move);  // returns false if move is illegal (defensive check)

// notation
bool parse_san(const Position *pos, const char *san, Move *out);
int  format_san(const Position *pos, const Move *move, char *buf, int bufsize);

// All formatting functions (position_to_fen, format_san, render_board) return
// bytes written excluding null terminator, or -1 on truncation.

// rules
typedef enum {
    STATUS_NORMAL,
    STATUS_CHECK,
    STATUS_CHECKMATE,
    STATUS_STALEMATE,
    STATUS_DRAW_50_MOVE,
    STATUS_DRAW_INSUFFICIENT,
} GameStatus;

GameStatus get_status(const Position *pos);
bool       is_square_attacked(const Position *pos, Square sq, int by_color);

// rendering
int render_board(const Position *pos, char *buf, int bufsize);
```

## Bitboard Operations

Built on C23 `<stdbit.h>`:

```c
#define popcount(bb)    stdc_count_ones(bb)
#define lsb(bb)         stdc_trailing_zeros(bb)
static inline void clear_lsb(Bitboard *bb) { *bb &= *bb - 1; }

#define rank_of(sq)     ((sq) >> 3)
#define file_of(sq)     ((sq) & 7)
#define square(r, f)    (((r) << 3) | (f))
#define bit(sq)         (1ULL << (sq))
```

Precomputed attack tables for non-sliding pieces:

```c
static Bitboard knight_attacks[64];
static Bitboard king_attacks[64];
static Bitboard pawn_attacks[2][64];
static Bitboard pawn_pushes[2][64]; // single-push targets only; double-push uses occupancy check at generation time
```

Sliding pieces (rook, bishop, queen) use **ray iteration** — walk each direction until hitting the board edge or a blocker. No magic bitboards, no precomputed sliding attack tables.

Magic bitboards are a lookup-table optimization that turns sliding piece attack generation into a single memory access via perfect hashing. Engines like Stockfish use them because search evaluates millions of positions per second and the per-lookup cost dominates. scb evaluates one position per invocation — ray iteration is correct, simple, requires zero precomputed tables, and adds zero startup cost. The complexity of magic bitboards (finding magic numbers, storing ~800KB of attack tables) would buy nothing here.

Similarly, scb omits precomputed `LineBB`, `BetweenBB`, and `SquareDistance` tables. These serve pin-aware move generation, static exchange evaluation (SEE), and positional heuristics — all search and evaluation concerns that are outside scb's scope.

## Move Generation

`is_square_attacked` is the sole attack-detection primitive. It answers "is square X attacked by color Y?" by querying each piece type's attack pattern from that square and checking for occupancy. Engines that search use richer primitives — pin masks, x-ray attacks, attack/defend maps — to enable move ordering, SEE, and evaluation without regenerating attacks. scb doesn't search, so a single general-purpose attack query is sufficient. Every check detection, castling legality test, and move legality filter routes through this one function.

Pseudo-legal generation + legality filter using copy-restore:

1. Generate all pseudo-legal moves for each piece type
2. For each candidate: copy the `Position` on the stack, call `make_move` on the copy, check if own king is attacked
3. If king is safe, the move is legal. Discard the copy either way.

Copy-restore is chosen over make/unmake. In search engines, make/unmake is standard because it avoids copying ~120 bytes millions of times per second — the engine makes a move, recurses, then undoes it in place using a saved undo struct. scb evaluates one position per invocation, so the copy cost is irrelevant. Copy-restore is simpler (no `unmake_move`, no undo struct, no state to restore on error) and avoids bugs where undo doesn't perfectly reverse the make.

Move count is threaded through generation functions via `int *n` parameter rather than file-scope static state — avoids shared state between calls.

Pawn generation uses bitboard shift operations with masks for promotion rank, double-push rank, and file edge clipping.

Special moves:
- **Castling** — king not in check, transit squares not attacked, castling rights intact
- **En passant** — validate via ep_square, filter for discovered check on own king
- **Promotion** — pawn on 7th generates 4 moves (Q, R, B, N)
- **Double push** — sets ep_square for opponent

## Rules & Game State

Stateless detection (from a single FEN):
- **Check** — king square attacked by opponent
- **Checkmate** — in check + zero legal moves
- **Stalemate** — not in check + zero legal moves
- **Insufficient material** — per FIDE Article 9.6: draw only when no sequence of legal moves can produce checkmate. In practice: K vs K, K+B vs K, K+N vs K, K+B vs K+B (same color bishops)
- **Fifty-move rule** — halfmove clock in FEN >= 100

History-dependent detection (consumer's responsibility):
- **Threefold repetition** — requires position history across moves. The consumer tracks FEN strings or position hashes and claims the draw when appropriate.
- **Move history / PGN** — scb returns the resulting FEN after each move. The consumer accumulates these into a game record if needed.

## SAN Parsing & Formatting

**Parsing** leans on the legal move list rather than implementing a full grammar parser. Extract piece type and target square from the SAN string, match against generated legal moves. Exactly one match = valid. Zero or multiple = error.

**Formatting** uses the position to determine disambiguation (file, rank, or both) and appends `+` or `#` for check/checkmate.

## CLI Interface

```
./chess new-game
./chess validate --fen <FEN> --move <SAN>
./chess legal-moves --fen <FEN>
./chess render --fen <FEN>
```

Exit codes:
- `0` — success
- `1` — invalid move (illegal or unparseable)
- `2` — game over (checkmate, stalemate, draw)
- `3` — bad input (malformed FEN, missing args)

Output uses `key: value` lines for agent parsing, followed by the ASCII board for human readability.

**`new-game` output:**
```
fen: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
status: normal

<board>
```

**`validate` output (success):**
```
fen: <new FEN after move>
status: normal|check|checkmate|stalemate|draw_50_move|draw_insufficient
move: <canonical SAN>

<board>
```

**`validate` output (failure):**
```
error: invalid move '<move>'
```

**`legal-moves` output:**
```
moves: e3 e4 d3 d4 Nf3 Nc3 ...
count: 20
```

**`render` output:**
```
fen: <echoed FEN>
status: normal|check|checkmate|stalemate|draw_50_move|draw_insufficient

<board>
```

## Board Rendering

```
  scb                              Move 12 · White to move

    a   b   c   d   e   f   g   h
  +---+---+---+---+---+---+---+---+
8 | r |   |   |   | k |   |   | r |
  +---+---+---+---+---+---+---+---+
7 | p | p |   |   |   | p | p | p |
  +---+---+---+---+---+---+---+---+
6 |   |   | n |   |   |   |   |   |
  +---+---+---+---+---+---+---+---+
5 |   |   |   | p |   |   |   |   |
  +---+---+---+---+---+---+---+---+
4 |   |   |   |   | P |   |   |   |
  +---+---+---+---+---+---+---+---+
3 |   |   |   |   |   | N |   |   |
  +---+---+---+---+---+---+---+---+
2 | P | P | P |   |   | P | P | P |
  +---+---+---+---+---+---+---+---+
1 | R |   |   |   | K |   |   | R |
  +---+---+---+---+---+---+---+---+

  Status: In progress (check)
  FEN: r3k2r/pp3ppp/2n5/3p4/4P3/5N2/PPP2PPP/R3K2R w KQkq - 2 12
```

Uppercase = white, lowercase = black, matching FEN convention. The renderer derives everything from the `Position` — move number from `fullmove`, side to move from `side`, status from `get_status`. History, captured pieces, and clocks are the caller's concern — consumers can wrap or augment the rendered output as needed.

## Deliverables

1. `libchess` — stateless C23 static library
2. `chess` CLI — thin frontend with 4 subcommands
3. `Makefile` — builds library and CLI, `-std=c23 -Wall -Wextra -Wpedantic -Werror`
4. `how-to-use.md` — usage guide for humans and agents
5. `README.md` — project overview with decision drivers section
