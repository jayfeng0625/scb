#include "test.h"
#include "chess.h"

void test_fen_starting(void) {
    Position pos;
    bool ok = position_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    ASSERT(ok);
    ASSERT_EQ(pos.side, WHITE);
    ASSERT_EQ(pos.castling, CASTLE_ALL);
    ASSERT_EQ(pos.ep_square, SQ_NONE);
    ASSERT_EQ(pos.halfmove, 0);
    ASSERT_EQ(pos.fullmove, 1);
    ASSERT_EQ(pos.pieces[WHITE][PAWN], 0xFF00ULL);
    ASSERT_EQ(pos.pieces[BLACK][PAWN], 0x00FF000000000000ULL);
    ASSERT(pos.pieces[WHITE][KING] & bit(E1));
    ASSERT(pos.pieces[BLACK][QUEEN] & bit(D8));
}

void test_fen_midgame(void) {
    Position pos;
    bool ok = position_from_fen(&pos, "r3k2r/pp3ppp/2n5/3p4/4P3/5N2/PPP2PPP/R3K2R b KQkq e3 2 12");
    ASSERT(ok);
    ASSERT_EQ(pos.side, BLACK);
    ASSERT_EQ(pos.ep_square, E3);
    ASSERT_EQ(pos.halfmove, 2);
    ASSERT_EQ(pos.fullmove, 12);
    ASSERT(pos.pieces[BLACK][KNIGHT] & bit(C6));
}

void test_fen_no_castling(void) {
    Position pos;
    bool ok = position_from_fen(&pos, "8/8/8/8/8/8/8/4K2k w - - 0 1");
    ASSERT(ok);
    ASSERT_EQ(pos.castling, 0);
}

void test_fen_invalid(void) {
    Position pos;
    ASSERT(!position_from_fen(&pos, ""));
    ASSERT(!position_from_fen(&pos, "garbage"));
    ASSERT(!position_from_fen(&pos, "8/8/8/8/8/8/8/8 w - - 0 1"));
}

void test_position_init(void) {
    Position pos;
    position_init(&pos);
    ASSERT_EQ(pos.side, WHITE);
    ASSERT_EQ(pos.castling, CASTLE_ALL);
    ASSERT(pos.pieces[WHITE][KING] & bit(E1));
    ASSERT(pos.pieces[BLACK][KING] & bit(E8));
}

void test_fen_roundtrip(void) {
    const char *fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/pp3ppp/2n5/3p4/4P3/5N2/PPP2PPP/R3K2R b KQkq e3 2 12",
        "8/8/8/8/8/8/8/4K2k w - - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
    };
    for (int i = 0; i < 4; i++) {
        Position pos;
        ASSERT(position_from_fen(&pos, fens[i]));
        char buf[FEN_MAX];
        position_to_fen(&pos, buf, sizeof(buf));
        ASSERT_STR(buf, fens[i]);
    }
}

void test_make_move_simple_pawn(void) {
    Position pos;
    position_init(&pos);
    Move m = { .from = E2, .to = E4, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][PAWN] & bit(E4));
    ASSERT(!(pos.pieces[WHITE][PAWN] & bit(E2)));
    ASSERT_EQ(pos.side, BLACK);
    ASSERT_EQ(pos.ep_square, E3);
}

void test_make_move_capture(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    Move m = { .from = E4, .to = D5, .piece = PAWN, .captured = PAWN,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][PAWN] & bit(D5));
    ASSERT(!(pos.pieces[BLACK][PAWN] & bit(D5)));
    ASSERT_EQ(pos.halfmove, 0);
}

void test_make_move_castling_kingside(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move m = { .from = E1, .to = G1, .piece = KING, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_KINGSIDE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][KING] & bit(G1));
    ASSERT(pos.pieces[WHITE][ROOK] & bit(F1));
    ASSERT(!(pos.pieces[WHITE][ROOK] & bit(H1)));
    ASSERT(!(pos.castling & (CASTLE_WK | CASTLE_WQ)));
}

void test_make_move_en_passant(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/pppp1ppp/8/4pP2/8/8/PPPPP1PP/RNBQKBNR w KQkq e6 0 3");
    Move m = { .from = F5, .to = E6, .piece = PAWN, .captured = PAWN,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = true };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][PAWN] & bit(E6));
    ASSERT(!(pos.pieces[BLACK][PAWN] & bit(E5)));
}

void test_make_move_promotion(void) {
    Position pos;
    position_from_fen(&pos, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Move m = { .from = E7, .to = E8, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = QUEEN, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][QUEEN] & bit(E8));
    ASSERT(!(pos.pieces[WHITE][PAWN] & bit(E7)));
    ASSERT(!(pos.pieces[WHITE][PAWN] & bit(E8)));
}

void test_make_move_castling_queenside(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move m = { .from = E1, .to = C1, .piece = KING, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_QUEENSIDE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[WHITE][KING] & bit(C1));
    ASSERT(pos.pieces[WHITE][ROOK] & bit(D1));
    ASSERT(!(pos.pieces[WHITE][ROOK] & bit(A1)));
    ASSERT(!(pos.castling & (CASTLE_WK | CASTLE_WQ)));
}

void test_make_move_black_castling(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1");
    Move m = { .from = E8, .to = G8, .piece = KING, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_KINGSIDE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[BLACK][KING] & bit(G8));
    ASSERT(pos.pieces[BLACK][ROOK] & bit(F8));
    ASSERT(!(pos.pieces[BLACK][ROOK] & bit(H8)));
    ASSERT(!(pos.castling & (CASTLE_BK | CASTLE_BQ)));
    ASSERT_EQ(pos.fullmove, 2);
}

void test_make_move_black_pawn(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    Move m = { .from = D7, .to = D5, .piece = PAWN, .captured = PIECE_NONE,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(pos.pieces[BLACK][PAWN] & bit(D5));
    ASSERT(!(pos.pieces[BLACK][PAWN] & bit(D7)));
    ASSERT_EQ(pos.side, WHITE);
    ASSERT_EQ(pos.ep_square, D6);
    ASSERT_EQ(pos.fullmove, 2);
}

void test_castling_revoked_by_rook_capture(void) {
    // White bishop captures black's h8 rook → black loses kingside castling
    Position pos;
    position_from_fen(&pos, "r3k2r/ppppppBp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move m = { .from = G7, .to = H8, .piece = BISHOP, .captured = ROOK,
               .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m));
    ASSERT(!(pos.castling & CASTLE_BK));   // black kingside revoked
    ASSERT(pos.castling & CASTLE_BQ);      // black queenside intact
    ASSERT(pos.castling & CASTLE_WK);      // white unaffected
    ASSERT(pos.castling & CASTLE_WQ);

    // Black bishop captures white's a1 rook → white loses queenside castling
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPbP/R3K2R b KQkq - 0 1");
    Move m2 = { .from = G2, .to = A1, .piece = BISHOP, .captured = ROOK,
                .promotion = PIECE_NONE, .castling = MOVE_CASTLE_NONE, .en_passant = false };
    ASSERT(make_move(&pos, &m2));
    ASSERT(!(pos.castling & CASTLE_WQ));   // white queenside revoked
    ASSERT(pos.castling & CASTLE_WK);      // white kingside intact
    ASSERT(pos.castling & CASTLE_BK);      // black unaffected
    ASSERT(pos.castling & CASTLE_BQ);
}

void test_fen_illegal_check(void) {
    Position pos;
    ASSERT(!position_from_fen(&pos, "4k3/8/8/8/4R3/8/8/4K3 w - - 0 1"));
}

void test_fen_rank_validation(void) {
    Position pos;
    // rank with only 7 squares (missing h8 piece)
    ASSERT(!position_from_fen(&pos, "rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    // rank with 9 squares (too many)
    ASSERT(!position_from_fen(&pos, "rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    // empty rank claiming 9 squares
    ASSERT(!position_from_fen(&pos, "rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    // rank with too many pieces and digits
    ASSERT(!position_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R1N1B1Q1K1 w KQkq - 0 1"));
    // only 6 ranks instead of 8
    ASSERT(!position_from_fen(&pos, "rnbqkbnr/pppppppp/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
}

void test_position_suite(void) {
    RUN_SUITE(test_fen_starting);
    RUN_SUITE(test_fen_midgame);
    RUN_SUITE(test_fen_no_castling);
    RUN_SUITE(test_fen_invalid);
    RUN_SUITE(test_fen_rank_validation);
    RUN_SUITE(test_position_init);
    RUN_SUITE(test_fen_roundtrip);
    RUN_SUITE(test_make_move_simple_pawn);
    RUN_SUITE(test_make_move_capture);
    RUN_SUITE(test_make_move_castling_kingside);
    RUN_SUITE(test_make_move_castling_queenside);
    RUN_SUITE(test_make_move_black_castling);
    RUN_SUITE(test_make_move_black_pawn);
    RUN_SUITE(test_make_move_en_passant);
    RUN_SUITE(test_make_move_promotion);
    RUN_SUITE(test_fen_illegal_check);
    RUN_SUITE(test_castling_revoked_by_rook_capture);
}
