#include "test.h"
#include "chess.h"

void test_status_normal(void) {
    Position pos;
    position_init(&pos);
    ASSERT_EQ(get_status(&pos), STATUS_NORMAL);
}

void test_status_check(void) {
    Position pos;
    position_from_fen(&pos, "4k3/8/8/8/4R3/8/8/4K3 b - - 0 1");
    ASSERT_EQ(get_status(&pos), STATUS_CHECK);
}

void test_status_checkmate(void) {
    Position pos;
    position_from_fen(&pos, "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4");
    ASSERT_EQ(get_status(&pos), STATUS_CHECKMATE);
}

void test_status_stalemate(void) {
    Position pos;
    position_from_fen(&pos, "k7/2Q5/1K6/8/8/8/8/8 b - - 0 1");
    ASSERT_EQ(get_status(&pos), STATUS_STALEMATE);
}

void test_status_draw_insufficient(void) {
    Position pos;
    position_from_fen(&pos, "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    ASSERT_EQ(get_status(&pos), STATUS_DRAW_INSUFFICIENT);
}

void test_status_draw_50_move(void) {
    Position pos;
    position_from_fen(&pos, "4k3/8/8/8/8/8/8/R3K3 w - - 100 50");
    ASSERT_EQ(get_status(&pos), STATUS_DRAW_50_MOVE);
}

void test_rules_suite(void) {
    RUN_SUITE(test_status_normal);
    RUN_SUITE(test_status_check);
    RUN_SUITE(test_status_checkmate);
    RUN_SUITE(test_status_stalemate);
    RUN_SUITE(test_status_draw_insufficient);
    RUN_SUITE(test_status_draw_50_move);
}
