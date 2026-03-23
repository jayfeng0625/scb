#include "test.h"
#include "chess.h"

void test_format_lan_pawn_push(void) {
    Move m = { .from = E2, .to = E4, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[LAN_MAX];
    format_lan(&m, buf, sizeof(buf));
    ASSERT_STR(buf, "e2e4");
}

void test_format_lan_knight(void) {
    Move m = { .from = G1, .to = F3, .piece = KNIGHT, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[LAN_MAX];
    format_lan(&m, buf, sizeof(buf));
    ASSERT_STR(buf, "g1f3");
}

void test_format_lan_castling(void) {
    Move m = { .from = E1, .to = G1, .piece = KING, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_KINGSIDE, .en_passant = false };
    char buf[LAN_MAX];
    format_lan(&m, buf, sizeof(buf));
    ASSERT_STR(buf, "e1g1");

    Move m2 = { .from = E1, .to = C1, .piece = KING, .captured = PIECE_NONE,
                .promotion = PIECE_NONE, .castling = MOVE_CASTLE_QUEENSIDE, .en_passant = false };
    format_lan(&m2, buf, sizeof(buf));
    ASSERT_STR(buf, "e1c1");
}

void test_format_lan_promotion(void) {
    Move m = { .from = E7, .to = E8, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = QUEEN, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[LAN_MAX];
    format_lan(&m, buf, sizeof(buf));
    ASSERT_STR(buf, "e7e8q");
}

void test_format_lan_capture_promotion(void) {
    Move m = { .from = D7, .to = E8, .piece = PAWN, .captured = ROOK,
               .promotion = KNIGHT, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    char buf[LAN_MAX];
    format_lan(&m, buf, sizeof(buf));
    ASSERT_STR(buf, "d7e8n");
}

void test_parse_lan_pawn(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(parse_lan(&pos, "e2e4", &m));
    ASSERT_EQ(m.from, E2);
    ASSERT_EQ(m.to, E4);
    ASSERT_EQ(m.piece, PAWN);
}

void test_parse_lan_knight(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(parse_lan(&pos, "g1f3", &m));
    ASSERT_EQ(m.from, G1);
    ASSERT_EQ(m.to, F3);
    ASSERT_EQ(m.piece, KNIGHT);
}

void test_parse_lan_castling(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move m;
    ASSERT(parse_lan(&pos, "e1g1", &m));
    ASSERT_EQ(m.castling, MOVE_CASTLE_KINGSIDE);
    ASSERT(parse_lan(&pos, "e1c1", &m));
    ASSERT_EQ(m.castling, MOVE_CASTLE_QUEENSIDE);
}

void test_parse_lan_capture(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    Move m;
    ASSERT(parse_lan(&pos, "e4d5", &m));
    ASSERT_EQ(m.to, D5);
    ASSERT_EQ(m.captured, PAWN);
}

void test_parse_lan_promotion(void) {
    Position pos;
    position_from_fen(&pos, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Move m;
    ASSERT(parse_lan(&pos, "e7e8q", &m));
    ASSERT_EQ(m.promotion, QUEEN);
    ASSERT_EQ(m.to, E8);
}

void test_parse_lan_en_passant(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/pppp1ppp/8/4pP2/8/8/PPPPP1PP/RNBQKBNR w KQkq e6 0 3");
    Move m;
    ASSERT(parse_lan(&pos, "f5e6", &m));
    ASSERT(m.en_passant);
    ASSERT_EQ(m.captured, PAWN);
}

void test_parse_lan_castling_gives_check(void) {
    // White castles kingside, rook lands on f1 which attacks black king on f8
    Position pos;
    position_from_fen(&pos, "5k2/8/8/8/8/8/8/4K2R w K - 0 1");
    Move m;
    ASSERT(parse_lan(&pos, "e1g1", &m));
    ASSERT_EQ(m.castling, MOVE_CASTLE_KINGSIDE);
    ASSERT(make_move(&pos, &m));
    ASSERT_EQ(get_status(&pos), STATUS_CHECK);
}

void test_parse_lan_spurious_promotion(void) {
    // e2e4q from starting position — valid squares, but not a promotion move
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(!parse_lan(&pos, "e2e4q", &m));
}

void test_parse_lan_invalid(void) {
    Position pos;
    position_init(&pos);
    Move m;
    ASSERT(!parse_lan(&pos, "e2e5", &m));   // illegal pawn move
    ASSERT(!parse_lan(&pos, "", &m));        // empty
    ASSERT(!parse_lan(&pos, "zz", &m));      // too short
    ASSERT(!parse_lan(&pos, "e2e4x", &m));   // bad promotion char
}

void test_notation_suite(void) {
    RUN_SUITE(test_format_lan_pawn_push);
    RUN_SUITE(test_format_lan_knight);
    RUN_SUITE(test_format_lan_castling);
    RUN_SUITE(test_format_lan_promotion);
    RUN_SUITE(test_format_lan_capture_promotion);
    RUN_SUITE(test_parse_lan_pawn);
    RUN_SUITE(test_parse_lan_knight);
    RUN_SUITE(test_parse_lan_castling);
    RUN_SUITE(test_parse_lan_capture);
    RUN_SUITE(test_parse_lan_promotion);
    RUN_SUITE(test_parse_lan_en_passant);
    RUN_SUITE(test_parse_lan_castling_gives_check);
    RUN_SUITE(test_parse_lan_spurious_promotion);
    RUN_SUITE(test_parse_lan_invalid);
}
