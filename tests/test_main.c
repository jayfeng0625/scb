#include "test.h"
#include "chess.h"

int test_passes = 0;
int test_fails = 0;

void test_bitboard_suite(void);
void test_position_suite(void);
void test_movegen_suite(void);
void test_notation_suite(void);
void test_rules_suite(void);
void test_render_suite(void);

int main(void) {
    printf("scb tests\n\n");
    bitboard_init();
    test_bitboard_suite();
    test_position_suite();
    test_movegen_suite();
    test_notation_suite();
    test_rules_suite();
    test_render_suite();
    TEST_REPORT();
}
