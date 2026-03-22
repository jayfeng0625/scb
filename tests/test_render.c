#include "test.h"
#include "chess.h"
#include <string.h>

void test_render_starting(void) {
    Position pos;
    position_init(&pos);
    char buf[RENDER_MAX];
    int n = render_board(&pos, buf, sizeof(buf));
    ASSERT(n > 0);
    ASSERT(strstr(buf, "scb"));
    ASSERT(strstr(buf, "White to move"));
    ASSERT(strstr(buf, "a   b   c   d   e   f   g   h"));
    ASSERT(strstr(buf, "| R | N | B | Q | K | B | N | R |"));
    ASSERT(strstr(buf, "| r | n | b | q | k | b | n | r |"));
    ASSERT(strstr(buf, "In progress"));
}

void test_render_check(void) {
    Position pos;
    position_from_fen(&pos, "4k3/8/8/8/4R3/8/8/4K3 b - - 0 1");
    char buf[RENDER_MAX];
    render_board(&pos, buf, sizeof(buf));
    ASSERT(strstr(buf, "In progress (check)"));
}

void test_render_suite(void) {
    RUN_SUITE(test_render_starting);
    RUN_SUITE(test_render_check);
}
