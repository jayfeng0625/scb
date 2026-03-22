#include "chess.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

static void recompute_occupancy(Position *pos) {
    for (int c = 0; c < 2; c++) {
        pos->occupied[c] = 0;
        for (int p = 0; p < 6; p++)
            pos->occupied[c] |= pos->pieces[c][p];
    }
    pos->all = pos->occupied[WHITE] | pos->occupied[BLACK];
}

int piece_at(const Position *pos, Square sq) {
    Bitboard b = bit(sq);
    for (int c = 0; c < 2; c++)
        for (int p = 0; p < 6; p++)
            if (pos->pieces[c][p] & b) return p;
    return PIECE_NONE;
}

int color_at(const Position *pos, Square sq) {
    Bitboard b = bit(sq);
    if (pos->occupied[WHITE] & b) return WHITE;
    if (pos->occupied[BLACK] & b) return BLACK;
    return -1;
}

bool position_from_fen(Position *pos, const char *fen) {
    if (!fen || !*fen) return false;
    memset(pos, 0, sizeof(*pos));
    pos->ep_square = SQ_NONE;

    const char *p = fen;
    int r = 7, f = 0;

    while (*p && *p != ' ') {
        if (*p == '/') {
            r--;
            f = 0;
        } else if (*p >= '1' && *p <= '8') {
            f += *p - '0';
        } else {
            int color = isupper(*p) ? WHITE : BLACK;
            int piece;
            switch (tolower(*p)) {
                case 'p': piece = PAWN; break;
                case 'n': piece = KNIGHT; break;
                case 'b': piece = BISHOP; break;
                case 'r': piece = ROOK; break;
                case 'q': piece = QUEEN; break;
                case 'k': piece = KING; break;
                default: return false;
            }
            if (r < 0 || r > 7 || f < 0 || f > 7) return false;
            pos->pieces[color][piece] |= bit(square(r, f));
            f++;
        }
        p++;
    }

    if (popcount(pos->pieces[WHITE][KING]) != 1) return false;
    if (popcount(pos->pieces[BLACK][KING]) != 1) return false;

    Bitboard rank18 = 0xFFULL | (0xFFULL << 56);
    if ((pos->pieces[WHITE][PAWN] | pos->pieces[BLACK][PAWN]) & rank18) return false;

    recompute_occupancy(pos);

    if (*p != ' ') return false;
    p++;

    if (*p == 'w') pos->side = WHITE;
    else if (*p == 'b') pos->side = BLACK;
    else return false;
    p++;

    if (*p != ' ') return false;
    p++;

    pos->castling = 0;
    if (*p == '-') {
        p++;
    } else {
        while (*p && *p != ' ') {
            switch (*p) {
                case 'K': pos->castling |= CASTLE_WK; break;
                case 'Q': pos->castling |= CASTLE_WQ; break;
                case 'k': pos->castling |= CASTLE_BK; break;
                case 'q': pos->castling |= CASTLE_BQ; break;
                default: return false;
            }
            p++;
        }
    }

    if (*p != ' ') return false;
    p++;

    if (*p == '-') {
        pos->ep_square = SQ_NONE;
        p++;
    } else {
        if (*p < 'a' || *p > 'h') return false;
        int ef = *p - 'a'; p++;
        if (*p < '1' || *p > '8') return false;
        int er = *p - '1'; p++;
        pos->ep_square = square(er, ef);
    }

    if (*p != ' ') return false;
    p++;

    pos->halfmove = atoi(p);
    while (*p && *p != ' ') p++;

    if (*p != ' ') return false;
    p++;

    pos->fullmove = atoi(p);

    int opponent = 1 - pos->side;
    Square opp_king = lsb(pos->pieces[opponent][KING]);
    if (is_square_attacked(pos, opp_king, pos->side)) return false;

    return true;
}

int position_to_fen(const Position *pos, char *buf, int bufsize) {
    char tmp[FEN_MAX];
    int n = 0;

    for (int r = 7; r >= 0; r--) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            Square sq = square(r, f);
            int p = piece_at(pos, sq);
            if (p == PIECE_NONE) {
                empty++;
            } else {
                if (empty) tmp[n++] = '0' + empty, empty = 0;
                const char chars[] = "pnbrqk";
                char c = chars[p];
                if (color_at(pos, sq) == WHITE) c = toupper(c);
                tmp[n++] = c;
            }
        }
        if (empty) tmp[n++] = '0' + empty;
        if (r > 0) tmp[n++] = '/';
    }

    tmp[n++] = ' ';
    tmp[n++] = pos->side == WHITE ? 'w' : 'b';
    tmp[n++] = ' ';

    if (pos->castling) {
        if (pos->castling & CASTLE_WK) tmp[n++] = 'K';
        if (pos->castling & CASTLE_WQ) tmp[n++] = 'Q';
        if (pos->castling & CASTLE_BK) tmp[n++] = 'k';
        if (pos->castling & CASTLE_BQ) tmp[n++] = 'q';
    } else {
        tmp[n++] = '-';
    }

    tmp[n++] = ' ';

    if (pos->ep_square == SQ_NONE) {
        tmp[n++] = '-';
    } else {
        tmp[n++] = 'a' + file_of(pos->ep_square);
        tmp[n++] = '1' + rank_of(pos->ep_square);
    }

    n += snprintf(tmp + n, sizeof(tmp) - n, " %d %d", pos->halfmove, pos->fullmove);
    tmp[n] = '\0';

    if (n >= bufsize) return -1;
    memcpy(buf, tmp, n + 1);
    return n;
}

bool make_move(Position *pos, const Move *move) {
    int us = pos->side;
    int them = 1 - us;

    pos->pieces[us][move->piece] &= ~bit(move->from);

    if (move->captured != PIECE_NONE) {
        if (move->en_passant) {
            Square cap_sq = us == WHITE ? move->to - 8 : move->to + 8;
            pos->pieces[them][PAWN] &= ~bit(cap_sq);
        } else {
            pos->pieces[them][move->captured] &= ~bit(move->to);
        }
    }

    int placed = (move->promotion != PIECE_NONE) ? move->promotion : move->piece;
    pos->pieces[us][placed] |= bit(move->to);

    if (move->castling == MOVE_CASTLE_KINGSIDE) {
        Square rook_from = us == WHITE ? H1 : H8;
        Square rook_to = us == WHITE ? F1 : F8;
        pos->pieces[us][ROOK] &= ~bit(rook_from);
        pos->pieces[us][ROOK] |= bit(rook_to);
    } else if (move->castling == MOVE_CASTLE_QUEENSIDE) {
        Square rook_from = us == WHITE ? A1 : A8;
        Square rook_to = us == WHITE ? D1 : D8;
        pos->pieces[us][ROOK] &= ~bit(rook_from);
        pos->pieces[us][ROOK] |= bit(rook_to);
    }

    if (move->piece == KING)
        pos->castling &= us == WHITE ? ~(CASTLE_WK | CASTLE_WQ) : ~(CASTLE_BK | CASTLE_BQ);
    if (move->from == A1 || move->to == A1) pos->castling &= ~CASTLE_WQ;
    if (move->from == H1 || move->to == H1) pos->castling &= ~CASTLE_WK;
    if (move->from == A8 || move->to == A8) pos->castling &= ~CASTLE_BQ;
    if (move->from == H8 || move->to == H8) pos->castling &= ~CASTLE_BK;

    if (move->piece == PAWN && abs(move->to - move->from) == 16)
        pos->ep_square = (move->from + move->to) / 2;
    else
        pos->ep_square = SQ_NONE;

    if (move->piece == PAWN || move->captured != PIECE_NONE)
        pos->halfmove = 0;
    else
        pos->halfmove++;

    if (us == BLACK) pos->fullmove++;

    pos->side = them;
    recompute_occupancy(pos);
    return true;
}

void position_init(Position *pos) {
    position_from_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}
