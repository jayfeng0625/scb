#include "chess.h"
#include <string.h>

static const char promo_chars[] = "pnbrqk";

int format_lan(const Move *move, char *buf, int bufsize) {
    char tmp[LAN_MAX];
    int n = 0;
    tmp[n++] = 'a' + file_of(move->from);
    tmp[n++] = '1' + rank_of(move->from);
    tmp[n++] = 'a' + file_of(move->to);
    tmp[n++] = '1' + rank_of(move->to);
    if (move->promotion != PIECE_NONE)
        tmp[n++] = promo_chars[move->promotion];
    tmp[n] = '\0';
    if (n >= bufsize) return -1;
    memcpy(buf, tmp, n + 1);
    return n;
}

bool parse_lan(const Position *pos, const char *lan, Move *out) {
    if (!lan) return false;
    int len = (int)strlen(lan);
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
