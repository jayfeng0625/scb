#include "chess.h"
#include <stdio.h>
#include <string.h>

enum { EXIT_OK = 0, EXIT_INVALID_MOVE = 1, EXIT_GAME_OVER = 2, EXIT_BAD_INPUT = 3 };

static void usage(void) {
    fprintf(stderr, "usage: chess <command> [options]\n\n"
                    "commands:\n"
                    "  new-game                         start a new game\n"
                    "  validate --fen <FEN> --move <SAN> validate and apply a move\n"
                    "  legal-moves --fen <FEN>           list legal moves\n"
                    "  render --fen <FEN>                render the board\n");
}

static const char *find_arg(int argc, char **argv, const char *name) {
    for (int i = 0; i < argc - 1; i++)
        if (strcmp(argv[i], name) == 0) return argv[i + 1];
    return NULL;
}

static int cmd_new_game(void) {
    Position pos;
    position_init(&pos);
    char fen[FEN_MAX];
    position_to_fen(&pos, fen, sizeof(fen));
    printf("fen: %s\n", fen);
    printf("status: normal\n\n");
    char board[RENDER_MAX];
    render_board(&pos, board, sizeof(board));
    printf("%s", board);
    return EXIT_OK;
}

static int cmd_validate(int argc, char **argv) {
    const char *fen_str = find_arg(argc, argv, "--fen");
    const char *move_str = find_arg(argc, argv, "--move");
    if (!fen_str || !move_str) { usage(); return EXIT_BAD_INPUT; }

    Position pos;
    if (!position_from_fen(&pos, fen_str)) {
        fprintf(stderr, "error: invalid FEN\n");
        return EXIT_BAD_INPUT;
    }

    Move move;
    if (!parse_san(&pos, move_str, &move)) {
        printf("error: invalid move '%s'\n", move_str);
        return EXIT_INVALID_MOVE;
    }

    char san[SAN_MAX];
    format_san(&pos, &move, san, sizeof(san));
    make_move(&pos, &move);

    GameStatus status = get_status(&pos);
    char fen[FEN_MAX];
    position_to_fen(&pos, fen, sizeof(fen));

    printf("fen: %s\n", fen);
    printf("status: %s\n", status_name(status));
    printf("move: %s\n\n", san);

    char board[RENDER_MAX];
    render_board(&pos, board, sizeof(board));
    printf("%s", board);

    if (status == STATUS_CHECKMATE || status == STATUS_STALEMATE ||
        status == STATUS_DRAW_50_MOVE || status == STATUS_DRAW_INSUFFICIENT)
        return EXIT_GAME_OVER;
    return EXIT_OK;
}

static int cmd_legal_moves(int argc, char **argv) {
    const char *fen_str = find_arg(argc, argv, "--fen");
    if (!fen_str) { usage(); return EXIT_BAD_INPUT; }

    Position pos;
    if (!position_from_fen(&pos, fen_str)) {
        fprintf(stderr, "error: invalid FEN\n");
        return EXIT_BAD_INPUT;
    }

    Move moves[MOVES_MAX];
    int count = generate_legal_moves(&pos, moves);

    printf("moves:");
    for (int i = 0; i < count; i++) {
        char san[SAN_MAX];
        format_san(&pos, &moves[i], san, sizeof(san));
        printf(" %s", san);
    }
    printf("\ncount: %d\n", count);
    return EXIT_OK;
}

static int cmd_render(int argc, char **argv) {
    const char *fen_str = find_arg(argc, argv, "--fen");
    if (!fen_str) { usage(); return EXIT_BAD_INPUT; }

    Position pos;
    if (!position_from_fen(&pos, fen_str)) {
        fprintf(stderr, "error: invalid FEN\n");
        return EXIT_BAD_INPUT;
    }

    GameStatus status = get_status(&pos);
    char fen[FEN_MAX];
    position_to_fen(&pos, fen, sizeof(fen));

    printf("fen: %s\n", fen);
    printf("status: %s\n\n", status_name(status));

    char board[RENDER_MAX];
    render_board(&pos, board, sizeof(board));
    printf("%s", board);
    return EXIT_OK;
}

int main(int argc, char **argv) {
    bitboard_init();

    if (argc < 2) { usage(); return EXIT_BAD_INPUT; }

    if (strcmp(argv[1], "new-game") == 0) return cmd_new_game();
    if (strcmp(argv[1], "validate") == 0) return cmd_validate(argc, argv);
    if (strcmp(argv[1], "legal-moves") == 0) return cmd_legal_moves(argc, argv);
    if (strcmp(argv[1], "render") == 0) return cmd_render(argc, argv);

    usage();
    return EXIT_BAD_INPUT;
}
