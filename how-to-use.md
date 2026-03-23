# How to Use scb

## For Humans

### Starting a game

```sh
./build/chess new-game
```

This prints the starting FEN and renders the board. Copy the FEN for your next move.

### Making a move

```sh
./build/chess validate \
    --fen "<FEN from previous output>" \
    --move "e2e4"
```

Moves use Long Algebraic Notation (LAN) — source square followed by destination square:
- Pawns: `e2e4`, `d7d5`, `e4d5` (capture), `e7e8q` (promotion)
- Pieces: `g1f3`, `f1c4`, `d1h5`, `e1g1` (kingside castle), `e1c1` (queenside)

The output gives you the new FEN, game status, and board. Use the new FEN for your next call.

### Understanding output

- `fen:` -- the position after the move, in Forsyth-Edwards Notation
- `status:` -- one of `normal`, `check`, `checkmate`, `stalemate`, `draw_50_move`, `draw_insufficient`
- `move:` -- the move in LAN (e.g. `e2e4`, `g1f3`, `e7e8q`)

### Seeing legal moves

```sh
./build/chess legal-moves --fen "<FEN>"
```

Lists all legal moves in LAN and a count.

## For Agents

### Interface

The CLI is stateless. Every call takes a FEN string as input and returns structured key-value output on stdout. No server process, no session state.

Four subcommands:

| Command | Required args | Description |
|---------|--------------|-------------|
| `new-game` | none | Returns starting position |
| `validate` | `--fen <FEN> --move <LAN>` | Applies a move, returns new position |
| `legal-moves` | `--fen <FEN>` | Lists all legal moves |
| `render` | `--fen <FEN>` | Renders the board as text |

### Output format

Output is line-oriented key-value pairs, easy to parse:

```
fen: <FEN string>
status: <status>
move: <LAN>
```

Parse by splitting each line on the first `: `. The board rendering follows after a blank line (for `new-game`, `validate`, and `render`).

### Exit codes

| Code | Meaning |
|------|---------|
| 0 | Success, game continues |
| 1 | Invalid move (move was not legal) |
| 2 | Game over (checkmate, stalemate, or draw) |
| 3 | Bad input (missing args, invalid FEN) |

### Agent workflow

A typical game loop:

1. **Start:** call `new-game`, parse the `fen:` value
2. **Move:** call `validate --fen <FEN> --move <LAN>`, parse the new `fen:` and `status:`
3. **Check status:** if exit code is 2, the game is over; if 1, the move was illegal -- pick another
4. **Repeat** step 2 with the new FEN

Example -- playing `1. e4 e5 2. Nf3`:

```sh
FEN=$(./build/chess new-game | head -1 | cut -d' ' -f2-)

OUT=$(./build/chess validate --fen "$FEN" --move "e2e4")
FEN=$(echo "$OUT" | head -1 | cut -d' ' -f2-)

OUT=$(./build/chess validate --fen "$FEN" --move "e7e5")
FEN=$(echo "$OUT" | head -1 | cut -d' ' -f2-)

OUT=$(./build/chess validate --fen "$FEN" --move "g1f3")
FEN=$(echo "$OUT" | head -1 | cut -d' ' -f2-)
```

### Tips for agents

- Always check the exit code before parsing output
- Use `legal-moves` to enumerate options before choosing
- The `fen:` line is always the first line of output for `new-game`, `validate`, and `render`
- FEN strings contain spaces -- quote them in shell commands
