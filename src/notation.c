#include "chess.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static const char piece_chars[] = "PNBRQK";

static int char_to_piece(char c) {
    for (int i = 0; i < 6; i++)
        if (piece_chars[i] == c) return i;
    return PIECE_NONE;
}

bool parse_san(const Position *pos, const char *san, Move *out) {
    if (!san || !*san) return false;

    Move moves[MOVES_MAX];
    int count = generate_legal_moves(pos, moves);

    const char *p = san;

    if (strncmp(p, "O-O-O", 5) == 0 || strncmp(p, "0-0-0", 5) == 0) {
        for (int i = 0; i < count; i++)
            if (moves[i].castling == MOVE_CASTLE_QUEENSIDE) { *out = moves[i]; return true; }
        return false;
    }
    if (strncmp(p, "O-O", 3) == 0 || strncmp(p, "0-0", 3) == 0) {
        for (int i = 0; i < count; i++)
            if (moves[i].castling == MOVE_CASTLE_KINGSIDE) { *out = moves[i]; return true; }
        return false;
    }

    int piece = PAWN;
    int from_file = -1, from_rank = -1;
    int to_file = -1, to_rank = -1;
    int promo = PIECE_NONE;

    if (isupper(*p) && *p != 'O') {
        piece = char_to_piece(*p);
        if (piece == PIECE_NONE) return false;
        p++;
    }

    char clean[8];
    int n = 0;
    for (const char *c = p; *c && n < 7; c++) {
        if (*c == 'x' || *c == '+' || *c == '#') continue;
        clean[n++] = *c;
    }
    clean[n] = '\0';

    if (n >= 3 && clean[n - 2] == '=') {
        promo = char_to_piece(clean[n - 1]);
        n -= 2;
        clean[n] = '\0';
    }

    if (n < 2) return false;
    to_file = clean[n - 2] - 'a';
    to_rank = clean[n - 1] - '1';
    if (to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) return false;
    Square target = square(to_rank, to_file);

    for (int i = 0; i < n - 2; i++) {
        if (clean[i] >= 'a' && clean[i] <= 'h') from_file = clean[i] - 'a';
        else if (clean[i] >= '1' && clean[i] <= '8') from_rank = clean[i] - '1';
    }

    Move *match = NULL;
    int matches = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].piece != piece) continue;
        if (moves[i].to != target) continue;
        if (promo != PIECE_NONE && moves[i].promotion != promo) continue;
        if (promo == PIECE_NONE && moves[i].promotion != PIECE_NONE) continue;
        if (from_file >= 0 && file_of(moves[i].from) != from_file) continue;
        if (from_rank >= 0 && rank_of(moves[i].from) != from_rank) continue;
        match = &moves[i];
        matches++;
    }

    if (matches != 1) return false;
    *out = *match;
    return true;
}

int format_san(const Position *pos, const Move *move, char *buf, int bufsize) {
    char tmp[SAN_MAX];
    int n = 0;

    if (move->castling == MOVE_CASTLE_KINGSIDE) {
        n = snprintf(tmp, sizeof(tmp), "O-O");
    } else if (move->castling == MOVE_CASTLE_QUEENSIDE) {
        n = snprintf(tmp, sizeof(tmp), "O-O-O");
    } else {
        if (move->piece != PAWN)
            tmp[n++] = piece_chars[move->piece];

        if (move->piece != PAWN) {
            Move moves[MOVES_MAX];
            int count = generate_legal_moves(pos, moves);
            bool need_file = false, need_rank = false;
            for (int i = 0; i < count; i++) {
                if (moves[i].piece == move->piece && moves[i].to == move->to &&
                    moves[i].from != move->from) {
                    if (file_of(moves[i].from) == file_of(move->from)) need_rank = true;
                    else need_file = true;
                }
            }
            if (need_file) tmp[n++] = 'a' + file_of(move->from);
            if (need_rank) tmp[n++] = '1' + rank_of(move->from);
        }

        if (move->piece == PAWN && move->captured != PIECE_NONE)
            tmp[n++] = 'a' + file_of(move->from);

        if (move->captured != PIECE_NONE)
            tmp[n++] = 'x';

        tmp[n++] = 'a' + file_of(move->to);
        tmp[n++] = '1' + rank_of(move->to);

        if (move->promotion != PIECE_NONE) {
            tmp[n++] = '=';
            tmp[n++] = piece_chars[move->promotion];
        }
    }

    Position copy = *pos;
    make_move(&copy, move);
    Move legal[MOVES_MAX];
    int legal_count = generate_legal_moves(&copy, legal);
    Square king_sq = lsb(copy.pieces[copy.side][KING]);
    if (is_square_attacked(&copy, king_sq, 1 - copy.side)) {
        tmp[n++] = (legal_count == 0) ? '#' : '+';
    }

    tmp[n] = '\0';
    if (n >= bufsize) return -1;
    memcpy(buf, tmp, n + 1);
    return n;
}
