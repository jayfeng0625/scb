#include "chess.h"
#include <stdio.h>
#include <string.h>

const char *status_name(GameStatus s) {
    static const char *names[] = {
        "normal", "check", "checkmate", "stalemate", "draw_50_move", "draw_insufficient"
    };
    if (s >= 0 && s <= STATUS_DRAW_INSUFFICIENT) return names[s];
    return "unknown";
}

static const char *status_display(GameStatus s) {
    switch (s) {
        case STATUS_NORMAL:             return "In progress";
        case STATUS_CHECK:              return "In progress (check)";
        case STATUS_CHECKMATE:          return "Checkmate";
        case STATUS_STALEMATE:          return "Stalemate";
        case STATUS_DRAW_50_MOVE:       return "Draw (50-move rule)";
        case STATUS_DRAW_INSUFFICIENT:  return "Draw (insufficient material)";
    }
    return "Unknown";
}

static char piece_char(const Position *pos, Square sq) {
    int p = piece_at(pos, sq);
    if (p == PIECE_NONE) return ' ';
    const char chars[] = "pnbrqk";
    char c = chars[p];
    if (color_at(pos, sq) == WHITE) c = c - 32;
    return c;
}

int render_board(const Position *pos, char *buf, int bufsize) {
    char tmp[RENDER_MAX];
    int n = 0;

    GameStatus status = get_status(pos);

    n += snprintf(tmp + n, sizeof(tmp) - n,
        "  scb%*sMove %d · %s to move\n\n",
        31, "", pos->fullmove, pos->side == WHITE ? "White" : "Black");

    n += snprintf(tmp + n, sizeof(tmp) - n,
        "    a   b   c   d   e   f   g   h\n");

    for (int r = 7; r >= 0; r--) {
        n += snprintf(tmp + n, sizeof(tmp) - n,
            "  +---+---+---+---+---+---+---+---+\n");
        n += snprintf(tmp + n, sizeof(tmp) - n, "%d |", r + 1);
        for (int f = 0; f < 8; f++) {
            n += snprintf(tmp + n, sizeof(tmp) - n,
                " %c |", piece_char(pos, square(r, f)));
        }
        n += snprintf(tmp + n, sizeof(tmp) - n, "\n");
    }
    n += snprintf(tmp + n, sizeof(tmp) - n,
        "  +---+---+---+---+---+---+---+---+\n");

    n += snprintf(tmp + n, sizeof(tmp) - n, "\n  Status: %s\n", status_display(status));

    char fen[FEN_MAX];
    position_to_fen(pos, fen, sizeof(fen));
    n += snprintf(tmp + n, sizeof(tmp) - n, "  FEN: %s\n", fen);

    if (n >= bufsize) return -1;
    memcpy(buf, tmp, n + 1);
    return n;
}
