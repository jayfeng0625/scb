#include "chess.h"

Bitboard knight_attacks[64];
Bitboard king_attacks[64];
Bitboard pawn_attacks[2][64];
Bitboard pawn_pushes[2][64];

static const int knight_offsets[8][2] = {
    {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}
};

static const int king_offsets[8][2] = {
    {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
};

static Bitboard offset_attacks(Square sq, const int offsets[][2], int count) {
    Bitboard result = 0;
    int r = rank_of(sq), f = file_of(sq);
    for (int i = 0; i < count; i++) {
        int nr = r + offsets[i][0], nf = f + offsets[i][1];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
            result |= bit(square(nr, nf));
    }
    return result;
}

static Bitboard ray_attacks(Square sq, int dr, int df, Bitboard occupied) {
    Bitboard result = 0;
    int r = rank_of(sq), f = file_of(sq);
    for (;;) {
        r += dr;
        f += df;
        if (r < 0 || r > 7 || f < 0 || f > 7) break;
        Square s = square(r, f);
        result |= bit(s);
        if (occupied & bit(s)) break;
    }
    return result;
}

Bitboard rook_attacks(Square sq, Bitboard occupied) {
    return ray_attacks(sq, 1, 0, occupied) | ray_attacks(sq, -1, 0, occupied) |
           ray_attacks(sq, 0, 1, occupied) | ray_attacks(sq, 0, -1, occupied);
}

Bitboard bishop_attacks(Square sq, Bitboard occupied) {
    return ray_attacks(sq, 1, 1, occupied) | ray_attacks(sq, 1, -1, occupied) |
           ray_attacks(sq, -1, 1, occupied) | ray_attacks(sq, -1, -1, occupied);
}

Bitboard queen_attacks(Square sq, Bitboard occupied) {
    return rook_attacks(sq, occupied) | bishop_attacks(sq, occupied);
}

void bitboard_init(void) {
    for (Square sq = A1; sq <= H8; sq++) {
        knight_attacks[sq] = offset_attacks(sq, knight_offsets, 8);
        king_attacks[sq] = offset_attacks(sq, king_offsets, 8);

        int r = rank_of(sq), f = file_of(sq);
        pawn_attacks[WHITE][sq] = 0;
        pawn_attacks[BLACK][sq] = 0;
        if (r < 7) {
            if (f > 0) pawn_attacks[WHITE][sq] |= bit(square(r + 1, f - 1));
            if (f < 7) pawn_attacks[WHITE][sq] |= bit(square(r + 1, f + 1));
        }
        if (r > 0) {
            if (f > 0) pawn_attacks[BLACK][sq] |= bit(square(r - 1, f - 1));
            if (f < 7) pawn_attacks[BLACK][sq] |= bit(square(r - 1, f + 1));
        }

        pawn_pushes[WHITE][sq] = (r >= 1 && r <= 6) ? bit(square(r + 1, f)) : 0;
        pawn_pushes[BLACK][sq] = (r >= 1 && r <= 6) ? bit(square(r - 1, f)) : 0;
    }
}
