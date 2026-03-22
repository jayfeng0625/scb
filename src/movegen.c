#include "chess.h"

bool is_square_attacked(const Position *pos, Square sq, int by_color) {
    if (knight_attacks[sq] & pos->pieces[by_color][KNIGHT]) return true;
    if (king_attacks[sq] & pos->pieces[by_color][KING]) return true;
    if (pawn_attacks[1 - by_color][sq] & pos->pieces[by_color][PAWN]) return true;

    Bitboard rq = pos->pieces[by_color][ROOK] | pos->pieces[by_color][QUEEN];
    if (rook_attacks(sq, pos->all) & rq) return true;

    Bitboard bq = pos->pieces[by_color][BISHOP] | pos->pieces[by_color][QUEEN];
    if (bishop_attacks(sq, pos->all) & bq) return true;

    return false;
}

static void add_move(Move *moves, int *n, Square from, Square to, int piece,
                     int captured, int promotion, int castling, bool en_passant) {
    moves[(*n)++] = (Move){
        .from = from, .to = to, .piece = piece,
        .captured = captured, .promotion = promotion,
        .castling = castling, .en_passant = en_passant
    };
}

static void add_promo_moves(Move *moves, int *n, Square from, Square to, int captured) {
    add_move(moves, n, from, to, PAWN, captured, QUEEN, MOVE_CASTLE_NONE, false);
    add_move(moves, n, from, to, PAWN, captured, ROOK, MOVE_CASTLE_NONE, false);
    add_move(moves, n, from, to, PAWN, captured, BISHOP, MOVE_CASTLE_NONE, false);
    add_move(moves, n, from, to, PAWN, captured, KNIGHT, MOVE_CASTLE_NONE, false);
}

static void generate_pawn_moves(const Position *pos, Move *moves, int *n) {
    int us = pos->side;
    int them = 1 - us;
    int promo_rank = us == WHITE ? 7 : 0;
    int double_rank = us == WHITE ? 1 : 6;
    int push_dir = us == WHITE ? 8 : -8;

    Bitboard pawns = pos->pieces[us][PAWN];
    while (pawns) {
        Square sq = lsb(pawns);
        clear_lsb(&pawns);

        Square push = sq + push_dir;
        if (push >= A1 && push <= H8 && !(pos->all & bit(push))) {
            if (rank_of(push) == promo_rank)
                add_promo_moves(moves, n, sq, push, PIECE_NONE);
            else
                add_move(moves, n, sq, push, PAWN, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_NONE, false);

            if (rank_of(sq) == double_rank) {
                Square dpush = push + push_dir;
                if (!(pos->all & bit(dpush)))
                    add_move(moves, n, sq, dpush, PAWN, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_NONE, false);
            }
        }

        Bitboard caps = pawn_attacks[us][sq] & pos->occupied[them];
        while (caps) {
            Square to = lsb(caps);
            clear_lsb(&caps);
            int captured = piece_at(pos, to);
            if (rank_of(to) == promo_rank)
                add_promo_moves(moves, n, sq, to, captured);
            else
                add_move(moves, n, sq, to, PAWN, captured, PIECE_NONE, MOVE_CASTLE_NONE, false);
        }

        if (pos->ep_square != SQ_NONE && (pawn_attacks[us][sq] & bit(pos->ep_square)))
            add_move(moves, n, sq, pos->ep_square, PAWN, PAWN, PIECE_NONE, MOVE_CASTLE_NONE, true);
    }
}

static void generate_piece_moves(const Position *pos, Move *moves, int *n, int piece_type) {
    int us = pos->side;
    Bitboard pieces = pos->pieces[us][piece_type];
    while (pieces) {
        Square sq = lsb(pieces);
        clear_lsb(&pieces);

        Bitboard attacks;
        switch (piece_type) {
            case KNIGHT: attacks = knight_attacks[sq]; break;
            case BISHOP: attacks = bishop_attacks(sq, pos->all); break;
            case ROOK:   attacks = rook_attacks(sq, pos->all); break;
            case QUEEN:  attacks = queen_attacks(sq, pos->all); break;
            default: attacks = 0; break;
        }
        attacks &= ~pos->occupied[us];

        while (attacks) {
            Square to = lsb(attacks);
            clear_lsb(&attacks);
            int captured = piece_at(pos, to);
            add_move(moves, n, sq, to, piece_type, captured, PIECE_NONE, MOVE_CASTLE_NONE, false);
        }
    }
}

static void generate_king_moves(const Position *pos, Move *moves, int *n) {
    int us = pos->side;
    int them = 1 - us;
    Square sq = lsb(pos->pieces[us][KING]);

    Bitboard attacks = king_attacks[sq] & ~pos->occupied[us];
    while (attacks) {
        Square to = lsb(attacks);
        clear_lsb(&attacks);
        int captured = piece_at(pos, to);
        add_move(moves, n, sq, to, KING, captured, PIECE_NONE, MOVE_CASTLE_NONE, false);
    }

    if (is_square_attacked(pos, sq, them)) return;

    if (us == WHITE) {
        if ((pos->castling & CASTLE_WK) &&
            !(pos->all & (bit(F1) | bit(G1))) &&
            !is_square_attacked(pos, F1, them))
            add_move(moves, n, E1, G1, KING, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_KINGSIDE, false);
        if ((pos->castling & CASTLE_WQ) &&
            !(pos->all & (bit(B1) | bit(C1) | bit(D1))) &&
            !is_square_attacked(pos, D1, them))
            add_move(moves, n, E1, C1, KING, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_QUEENSIDE, false);
    } else {
        if ((pos->castling & CASTLE_BK) &&
            !(pos->all & (bit(F8) | bit(G8))) &&
            !is_square_attacked(pos, F8, them))
            add_move(moves, n, E8, G8, KING, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_KINGSIDE, false);
        if ((pos->castling & CASTLE_BQ) &&
            !(pos->all & (bit(B8) | bit(C8) | bit(D8))) &&
            !is_square_attacked(pos, D8, them))
            add_move(moves, n, E8, C8, KING, PIECE_NONE, PIECE_NONE, MOVE_CASTLE_QUEENSIDE, false);
    }
}

int generate_legal_moves(const Position *pos, Move *moves) {
    Move pseudo[MOVES_MAX];
    int n = 0;

    generate_pawn_moves(pos, pseudo, &n);
    generate_piece_moves(pos, pseudo, &n, KNIGHT);
    generate_piece_moves(pos, pseudo, &n, BISHOP);
    generate_piece_moves(pos, pseudo, &n, ROOK);
    generate_piece_moves(pos, pseudo, &n, QUEEN);
    generate_king_moves(pos, pseudo, &n);

    int legal = 0;
    for (int i = 0; i < n; i++) {
        Position copy = *pos;
        make_move(&copy, &pseudo[i]);
        Square king_sq = lsb(copy.pieces[pos->side][KING]);
        if (!is_square_attacked(&copy, king_sq, copy.side))
            moves[legal++] = pseudo[i];
    }
    return legal;
}
