#include "chess.h"

static bool insufficient_material(const Position *pos) {
    for (int c = 0; c < 2; c++) {
        if (pos->pieces[c][PAWN]) return false;
        if (pos->pieces[c][ROOK]) return false;
        if (pos->pieces[c][QUEEN]) return false;
    }
    int wn = popcount(pos->pieces[WHITE][KNIGHT]);
    int wb = popcount(pos->pieces[WHITE][BISHOP]);
    int bn = popcount(pos->pieces[BLACK][KNIGHT]);
    int bb = popcount(pos->pieces[BLACK][BISHOP]);

    int w_minor = wn + wb;
    int b_minor = bn + bb;

    if (w_minor == 0 && b_minor == 0) return true;
    if (w_minor <= 1 && b_minor == 0) return true;
    if (b_minor <= 1 && w_minor == 0) return true;
    if (wn == 0 && bn == 0 && wb == 1 && bb == 1) {
        Square wsq = lsb(pos->pieces[WHITE][BISHOP]);
        Square bsq = lsb(pos->pieces[BLACK][BISHOP]);
        if (((rank_of(wsq) + file_of(wsq)) % 2) == ((rank_of(bsq) + file_of(bsq)) % 2))
            return true;
    }
    return false;
}

GameStatus get_status(const Position *pos) {
    if (pos->halfmove >= 100) return STATUS_DRAW_50_MOVE;
    if (insufficient_material(pos)) return STATUS_DRAW_INSUFFICIENT;

    Square king_sq = lsb(pos->pieces[pos->side][KING]);
    bool in_check = is_square_attacked(pos, king_sq, 1 - pos->side);

    Move moves[MOVES_MAX];
    int count = generate_legal_moves(pos, moves);

    if (count == 0)
        return in_check ? STATUS_CHECKMATE : STATUS_STALEMATE;
    return in_check ? STATUS_CHECK : STATUS_NORMAL;
}
