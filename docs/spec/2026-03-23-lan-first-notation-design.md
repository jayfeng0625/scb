# LAN-First Notation — Design Specification

Replace SAN (Standard Algebraic Notation) with LAN (Long Algebraic Notation) as the first-class move representation in `libchess`. Defer SAN to human-facing consumer layers.

## Rationale

`libchess` is a stateless rules engine designed for programmatic consumers — tool servers, agents, UIs, and potentially UCI frontends. Its current notation layer uses SAN, which is a human-readable format that adds parsing complexity without benefiting machine consumers.

**Why LAN is the right primitive:**

- **Self-describing.** `e2e4` encodes from-square and to-square directly. No position context needed to understand the move. A consumer can construct and interpret moves without any chess notation knowledge.
- **Easy to parse.** 4-5 characters, fixed format: `<from_file><from_rank><to_file><to_rank>[promotion]`. No disambiguation, no capture markers, no check suffixes.
- **Easy to format.** Encode `move.from` and `move.to` as coordinates. No legal move generation needed, no position context needed.
- **Maps directly to the Move struct.** `Move.from` and `Move.to` *are* LAN internally — the notation is just a string encoding of what we already have.
- **UCI-compatible.** If a UCI layer is ever built, it gets move parsing for free.

**Why SAN doesn't belong in the rules engine:**

- SAN exists for humans reading chess books and scoresheet notation. It's a display format.
- `parse_san` requires generating all legal moves just to resolve which piece is meant — 85 lines of pattern matching against a move list.
- `format_san` requires generating legal moves *twice* — once for disambiguation, once post-move for check/checkmate suffixes — 56 lines.
- This complexity serves no machine consumer. Every programmatic caller pays the cost of notation logic it doesn't need.
- Any layer that needs SAN for human display (chat UI, PGN export) can implement it on top of `libchess` using `generate_legal_moves` and position state. The implementation lives in git history if needed as reference.

## Move Format

LAN uses lowercase coordinates, matching UCI convention:

```
<from_file><from_rank><to_file><to_rank>[promotion]
```

| Move type         | Example   | Notes                                    |
|-------------------|-----------|------------------------------------------|
| Pawn push         | `e2e4`    |                                          |
| Piece move        | `g1f3`    | No piece letter prefix                   |
| Capture           | `e4d5`    | No `x` marker                            |
| Kingside castle   | `e1g1`    | King's actual movement, not `O-O`        |
| Queenside castle  | `e1c1`    | King's actual movement, not `O-O-O`      |
| Promotion         | `e7e8q`   | Lowercase piece suffix, no `=`           |
| Promotion capture | `d7e8n`   | Capture + underpromotion                 |
| En passant        | `e5d6`    | Looks like a normal capture              |

Promotion pieces: `q`, `r`, `b`, `n` (always lowercase).

## Buffer Constant

```c
#define LAN_MAX 6   // "e7e8q" + null = 6 bytes
```

Replaces `SAN_MAX 12`.

## API Changes

### Removed

```c
// notation.c — removed
bool parse_san(const Position *pos, const char *san, Move *out);
int  format_san(const Position *pos, const Move *move, char *buf, int bufsize);
```

### Added

```c
// notation.c — added
bool parse_lan(const Position *pos, const char *lan, Move *out);
int  format_lan(const Move *move, char *buf, int bufsize);
```

Key signature difference: `format_lan` does **not** take a `Position` parameter. It only needs the `Move` struct.

`parse_lan` still takes a `Position` because it needs to:
1. Look up `piece` and `captured` from the board (LAN doesn't encode piece type)
2. Detect en passant (pawn captures empty square where ep_square is set)
3. Detect castling (king moves two squares)
4. Validate the move is legal (generate legal moves, confirm match)

## Implementation

### `format_lan` (~10 lines)

```c
int format_lan(const Move *move, char *buf, int bufsize) {
    int n = 0;
    char tmp[LAN_MAX];
    tmp[n++] = 'a' + file_of(move->from);
    tmp[n++] = '1' + rank_of(move->from);
    tmp[n++] = 'a' + file_of(move->to);
    tmp[n++] = '1' + rank_of(move->to);
    if (move->promotion != PIECE_NONE) {
        const char promo_chars[] = "pnbrqk";
        tmp[n++] = promo_chars[move->promotion];
    }
    tmp[n] = '\0';
    if (n >= bufsize) return -1;
    memcpy(buf, tmp, n + 1);
    return n;
}
```

### `parse_lan` (~30 lines)

```c
bool parse_lan(const Position *pos, const char *lan, Move *out) {
    if (!lan) return false;
    int len = strlen(lan);
    if (len < 4 || len > 5) return false;

    int ff = lan[0] - 'a', fr = lan[1] - '1';
    int tf = lan[2] - 'a', tr = lan[3] - '1';
    if (ff < 0 || ff > 7 || fr < 0 || fr > 7 ||
        tf < 0 || tf > 7 || tr < 0 || tr > 7) return false;

    Square from = square(fr, ff);
    Square to   = square(tr, tf);

    int promo = PIECE_NONE;
    if (len == 5) {
        switch (lan[4]) {
        case 'q': promo = QUEEN;  break;
        case 'r': promo = ROOK;   break;
        case 'b': promo = BISHOP; break;
        case 'n': promo = KNIGHT; break;
        default:  return false;
        }
    }

    Move moves[MOVES_MAX];
    int count = generate_legal_moves(pos, moves);
    for (int i = 0; i < count; i++) {
        if (moves[i].from == from && moves[i].to == to &&
            moves[i].promotion == promo) {
            *out = moves[i];
            return true;
        }
    }
    return false;
}
```

## CLI Changes

```
./chess validate --fen <FEN> --move <LAN>
./chess legal-moves --fen <FEN>
```

- `validate` accepts LAN instead of SAN (e.g. `--move e2e4` instead of `--move e4`)
- `legal-moves` outputs LAN (e.g. `moves: e2e3 e2e4 d2d3 d2d4 g1f3 b1c3 ...`)
- `validate` echoes the applied move as LAN in its output (`move: e2e4`)
- Usage help updated to reflect LAN

## Test Changes

- Remove SAN-specific tests (parse/format disambiguation, castling notation, check suffixes)
- Add LAN parse/format tests covering: basic moves, castling as king movement, promotions, en passant, invalid input
- Existing position, movegen, and rules tests are unaffected — they construct `Move` structs directly

## What This Removes

| Removed                  | Lines | Reason                                      |
|--------------------------|-------|----------------------------------------------|
| `parse_san`              | ~85   | SAN is a presentation concern                |
| `format_san`             | ~56   | SAN is a presentation concern                |
| `char_to_piece` helper   | ~4    | Only used by SAN parser                      |
| `piece_chars` table      | ~1    | Only used by SAN formatter                   |
| `SAN_MAX` constant       | ~1    | Replaced by `LAN_MAX`                        |
| SAN test cases           | ~50   | Replaced by LAN test cases                   |
| **Total**                | ~197  |                                               |

## What This Adds

| Added                    | Lines | Notes                                        |
|--------------------------|-------|----------------------------------------------|
| `format_lan`             | ~10   | No position context needed                   |
| `parse_lan`              | ~30   | Coordinate decode + legal move match         |
| `LAN_MAX` constant       | ~1    |                                              |
| LAN test cases           | ~40   | Simpler than SAN tests                       |
| **Total**                | ~81   |                                               |

**Net: ~116 fewer lines across the codebase.**

## Unchanged

- `Position`, `Move`, `Bitboard` types
- `position_init`, `position_from_fen`, `position_to_fen`, `make_move`
- `generate_legal_moves`, `is_square_attacked`
- `get_status`, `render_board`
- All position, movegen, rules, and render tests
