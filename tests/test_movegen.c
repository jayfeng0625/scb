#include "test.h"
#include "chess.h"

void test_attacked_initial(void) {
    Position pos;
    position_init(&pos);
    ASSERT(is_square_attacked(&pos, D3, WHITE));
    ASSERT(is_square_attacked(&pos, F3, WHITE));
    ASSERT(!is_square_attacked(&pos, E4, WHITE));
    ASSERT(is_square_attacked(&pos, D6, BLACK));
}

void test_attacked_by_knight(void) {
    Position pos;
    position_from_fen(&pos, "8/8/8/8/4N3/8/8/4K2k w - - 0 1");
    ASSERT(is_square_attacked(&pos, F6, WHITE));
    ASSERT(is_square_attacked(&pos, D6, WHITE));
    ASSERT(is_square_attacked(&pos, C3, WHITE));
    ASSERT(!is_square_attacked(&pos, E5, WHITE));
}

void test_attacked_by_sliding(void) {
    Position pos;
    position_from_fen(&pos, "8/8/8/8/4R3/8/8/4K2k w - - 0 1");
    ASSERT(is_square_attacked(&pos, E8, WHITE));
    ASSERT(is_square_attacked(&pos, A4, WHITE));
    ASSERT(!is_square_attacked(&pos, D5, WHITE));
}

void test_legal_moves_starting(void) {
    Position pos;
    position_init(&pos);
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    ASSERT_EQ(count, 20);
}

void test_legal_moves_kiwipete(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    ASSERT_EQ(count, 48);
}

void test_legal_moves_promotions(void) {
    Position pos;
    position_from_fen(&pos, "8/4P3/8/8/8/8/8/4K2k w - - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    int promo_count = 0;
    for (int i = 0; i < count; i++)
        if (moves[i].promotion != PIECE_NONE) promo_count++;
    ASSERT_EQ(promo_count, 4);
}

void test_legal_moves_en_passant(void) {
    Position pos;
    position_from_fen(&pos, "rnbqkbnr/pppp1ppp/8/4pP2/8/8/PPPPP1PP/RNBQKBNR w KQkq e6 0 3");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    bool found_ep = false;
    for (int i = 0; i < count; i++)
        if (moves[i].en_passant) found_ep = true;
    ASSERT(found_ep);
}

void test_legal_moves_castling(void) {
    Position pos;
    position_from_fen(&pos, "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    int castle_count = 0;
    for (int i = 0; i < count; i++)
        if (moves[i].castling != MOVE_CASTLE_NONE) castle_count++;
    ASSERT_EQ(castle_count, 2);
}

void test_legal_moves_in_check(void) {
    Position pos;
    position_from_fen(&pos, "4k3/4r3/8/8/8/8/8/4K3 w - - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    for (int i = 0; i < count; i++) {
        ASSERT(moves[i].piece == KING);
    }
}

void test_scholars_mate(void) {
    Position pos;
    position_init(&pos);
    Move m;

    ASSERT(parse_lan(&pos, "e2e4", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "e7e5", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "f1c4", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "b8c6", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "d1h5", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "g8f6", &m)); ASSERT(make_move(&pos, &m));
    ASSERT(parse_lan(&pos, "h5f7", &m)); ASSERT(make_move(&pos, &m));

    ASSERT_EQ(get_status(&pos), STATUS_CHECKMATE);
}

void test_en_passant_two_pawns(void) {
    // Pawns on d5 and f5 can both capture en passant on e6
    Position pos;
    position_from_fen(&pos, "4k3/8/8/3PpP2/8/8/8/4K3 w - e6 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    int ep_count = 0;
    for (int i = 0; i < count; i++)
        if (moves[i].en_passant) ep_count++;
    ASSERT_EQ(ep_count, 2);
}

void test_castling_destination_attacked(void) {
    // Bishop h2 attacks g1 but not f1 or e1
    // Kingside castle illegal (king lands on attacked g1), queenside still OK
    Position pos;
    position_from_fen(&pos, "r3k2r/8/8/8/8/8/7b/R3K2R w KQkq - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    bool found_kingside = false, found_queenside = false;
    for (int i = 0; i < count; i++) {
        if (moves[i].castling == MOVE_CASTLE_KINGSIDE) found_kingside = true;
        if (moves[i].castling == MOVE_CASTLE_QUEENSIDE) found_queenside = true;
    }
    ASSERT(!found_kingside);
    ASSERT(found_queenside);
}

void test_pinned_piece(void) {
    // Knight e4 pinned to king e1 by rook e8 — no knight moves possible
    Position pos;
    position_from_fen(&pos, "4r3/8/8/8/4N3/8/8/4K2k w - - 0 1");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    for (int i = 0; i < count; i++)
        ASSERT(moves[i].piece != KNIGHT);

    // Rook e4 pinned along e-file — can slide along file, not off it
    position_from_fen(&pos, "4r3/8/8/8/4R3/8/8/4K2k w - - 0 1");
    count = generate_legal_moves(&pos, moves);
    bool rook_moves = false;
    for (int i = 0; i < count; i++) {
        if (moves[i].piece == ROOK) {
            rook_moves = true;
            ASSERT_EQ(file_of(moves[i].to), file_of(E4));
        }
    }
    ASSERT(rook_moves);
}

void test_double_check(void) {
    // King e1 in double check from rook e7 + bishop c3
    // Queen d1 could capture c3 or block e2, but neither resolves both
    Position pos;
    position_from_fen(&pos, "4k3/4r3/8/8/8/2b5/8/3QK3 w - - 0 1");
    ASSERT_EQ(get_status(&pos), STATUS_CHECK);
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    ASSERT(count > 0);
    for (int i = 0; i < count; i++)
        ASSERT_EQ(moves[i].piece, KING);
}

void test_en_passant_discovered_check(void) {
    // White king on a5, white pawn on d5, black pawn on c5 (just double-pushed),
    // black rook on h5. En passant dxc6 would remove the c5 pawn, exposing
    // the white king to the rook on h5 via rank 5. Move must be illegal.
    Position pos;
    position_from_fen(&pos, "4k3/8/8/K1pP3r/8/8/8/8 w - c6 0 2");
    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);
    for (int i = 0; i < count; i++) {
        ASSERT(!moves[i].en_passant);
    }
}

void test_movegen_suite(void) {
    RUN_SUITE(test_attacked_initial);
    RUN_SUITE(test_attacked_by_knight);
    RUN_SUITE(test_attacked_by_sliding);
    RUN_SUITE(test_legal_moves_starting);
    RUN_SUITE(test_legal_moves_kiwipete);
    RUN_SUITE(test_legal_moves_promotions);
    RUN_SUITE(test_legal_moves_en_passant);
    RUN_SUITE(test_legal_moves_castling);
    RUN_SUITE(test_legal_moves_in_check);
    RUN_SUITE(test_en_passant_two_pawns);
    RUN_SUITE(test_castling_destination_attacked);
    RUN_SUITE(test_pinned_piece);
    RUN_SUITE(test_double_check);
    RUN_SUITE(test_en_passant_discovered_check);
    RUN_SUITE(test_scholars_mate);
}
