#include "test.h"
#include "chess.h"

void test_parse_san_pawn(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(parse_san(&pos, "e4", &m));
    ASSERT_EQ(m.from, E2);
    ASSERT_EQ(m.to, E4);
    ASSERT_EQ(m.piece, PAWN);
}

void test_parse_san_knight(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(parse_san(&pos, "Nf3", &m));
    ASSERT_EQ(m.from, G1);
    ASSERT_EQ(m.to, F3);
    ASSERT_EQ(m.piece, KNIGHT);
}

void test_parse_san_castling(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move m;
    ASSERT(parse_san(&pos, "O-O", &m));
    ASSERT_EQ(m.castling, MOVE_CASTLE_KINGSIDE);
    ASSERT(parse_san(&pos, "O-O-O", &m));
    ASSERT_EQ(m.castling, MOVE_CASTLE_QUEENSIDE);
}

void test_parse_san_capture(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    Move m;
    ASSERT(parse_san(&pos, "exd5", &m));
    ASSERT_EQ(m.to, D5);
    ASSERT_EQ(m.captured, PAWN);
}

void test_parse_san_promotion(void) {
    Position pos;
    position_from_fen(&pos, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Move m;
    ASSERT(parse_san(&pos, "e8=Q", &m));
    ASSERT_EQ(m.promotion, QUEEN);
    ASSERT_EQ(m.to, E8);
}

void test_parse_san_invalid(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(!parse_san(&pos, "Ke9", &m));
    ASSERT(!parse_san(&pos, "", &m));
    ASSERT(!parse_san(&pos, "Qd4", &m));
}

void test_format_san(void) {
    Position pos;
    position_init(&pos);
    Move m = { .from = E2, .to = E4, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[SAN_MAX];
    format_san(&pos, &m, buf, sizeof(buf));
    ASSERT_STR(buf, "e4");
}

void test_format_san_knight(void) {
    Position pos;
    position_init(&pos);
    Move m = { .from = G1, .to = F3, .piece = KNIGHT, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[SAN_MAX];
    format_san(&pos, &m, buf, sizeof(buf));
    ASSERT_STR(buf, "Nf3");
}

void test_format_san_disambiguation(void) {
    Position pos;
    position_from_fen(&pos, "8/8/8/8/8/8/8/1N2KN1k w - - 0 1");
    Move m = { .from = B1, .to = D2, .piece = KNIGHT, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[SAN_MAX];
    format_san(&pos, &m, buf, sizeof(buf));
    ASSERT_STR(buf, "Nbd2");
}

void test_notation_suite(void) {
    RUN_SUITE(test_parse_san_pawn);
    RUN_SUITE(test_parse_san_knight);
    RUN_SUITE(test_parse_san_castling);
    RUN_SUITE(test_parse_san_capture);
    RUN_SUITE(test_parse_san_promotion);
    RUN_SUITE(test_parse_san_invalid);
    RUN_SUITE(test_format_san);
    RUN_SUITE(test_format_san_knight);
    RUN_SUITE(test_format_san_disambiguation);
}
