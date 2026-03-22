#include "test.h"
#include "chess.h"

void test_square_macros(void) {
    ASSERT_EQ(rank_of(A1), 0);
    ASSERT_EQ(file_of(A1), 0);
    ASSERT_EQ(rank_of(H8), 7);
    ASSERT_EQ(file_of(H8), 7);
    ASSERT_EQ(square(0, 0), A1);
    ASSERT_EQ(square(7, 7), H8);
    ASSERT_EQ(square(3, 4), E4);
    ASSERT_EQ(bit(A1), 1ULL);
    ASSERT_EQ(bit(H8), 1ULL << 63);
}

void test_popcount(void) {
    ASSERT_EQ(popcount((Bitboard)0), 0);
    ASSERT_EQ(popcount((Bitboard)0xFF), 8);
    ASSERT_EQ(popcount((Bitboard)0xFFFF000000000000ULL), 16);
}

void test_lsb_clear(void) {
    ASSERT_EQ(lsb((Bitboard)1ULL), 0);
    ASSERT_EQ(lsb((Bitboard)(1ULL << 63)), 63);
    Bitboard bb = 0x24;
    ASSERT_EQ(lsb(bb), 2);
    clear_lsb(&bb);
    ASSERT_EQ(lsb(bb), 5);
    clear_lsb(&bb);
    ASSERT_EQ(bb, 0ULL);
}

void test_knight_attacks(void) {
    Bitboard expected = bit(D2) | bit(F2) | bit(C3) | bit(G3) |
                        bit(C5) | bit(G5) | bit(D6) | bit(F6);
    ASSERT_EQ(knight_attacks[E4], expected);

    expected = bit(B3) | bit(C2);
    ASSERT_EQ(knight_attacks[A1], expected);
}

void test_king_attacks(void) {
    Bitboard expected = bit(D3) | bit(E3) | bit(F3) |
                        bit(D4) | bit(F4) |
                        bit(D5) | bit(E5) | bit(F5);
    ASSERT_EQ(king_attacks[E4], expected);

    expected = bit(B1) | bit(A2) | bit(B2);
    ASSERT_EQ(king_attacks[A1], expected);
}

void test_pawn_attacks(void) {
    ASSERT_EQ(pawn_attacks[WHITE][E4], bit(D5) | bit(F5));
    ASSERT_EQ(pawn_attacks[BLACK][E4], bit(D3) | bit(F3));
    ASSERT_EQ(pawn_attacks[WHITE][A2], bit(B3));
    ASSERT_EQ(pawn_attacks[BLACK][H7], bit(G6));
}

void test_rook_attacks(void) {
    Bitboard occ = 0;
    Bitboard atk = rook_attacks(A1, occ);
    ASSERT(atk & bit(A8));
    ASSERT(atk & bit(H1));
    ASSERT(!(atk & bit(A1)));
    ASSERT_EQ(popcount(atk), 14);

    occ = bit(E7) | bit(B4);
    atk = rook_attacks(E4, occ);
    ASSERT(atk & bit(E7));
    ASSERT(!(atk & bit(E8)));
    ASSERT(atk & bit(B4));
    ASSERT(!(atk & bit(A4)));
}

void test_bishop_attacks(void) {
    Bitboard occ = 0;
    Bitboard atk = bishop_attacks(D4, occ);
    ASSERT(atk & bit(A1));
    ASSERT(atk & bit(H8));
    ASSERT(atk & bit(A7));
    ASSERT(atk & bit(G1));
    ASSERT_EQ(popcount(atk), 13);
}

void test_queen_attacks(void) {
    Bitboard occ = 0;
    Bitboard q = queen_attacks(E4, occ);
    Bitboard r = rook_attacks(E4, occ);
    Bitboard b = bishop_attacks(E4, occ);
    ASSERT_EQ(q, r | b);
}

void test_bitboard_suite(void) {
    RUN_SUITE(test_square_macros);
    RUN_SUITE(test_popcount);
    RUN_SUITE(test_lsb_clear);
    RUN_SUITE(test_knight_attacks);
    RUN_SUITE(test_king_attacks);
    RUN_SUITE(test_pawn_attacks);
    RUN_SUITE(test_rook_attacks);
    RUN_SUITE(test_bishop_attacks);
    RUN_SUITE(test_queen_attacks);
}
