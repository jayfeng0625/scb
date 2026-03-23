#ifndef CHESS_H
#define CHESS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __has_include
#  if __has_include(<stdbit.h>)
#    include <stdbit.h>
#    define popcount(bb)  ((int)stdc_count_ones(bb))
#    define lsb(bb)       stdc_trailing_zeros(bb)
#  endif
#endif

#ifndef popcount
#  define popcount(bb)  ((int)__builtin_popcountll(bb))
#  define lsb(bb)       ((int)__builtin_ctzll(bb))
#endif

typedef uint64_t Bitboard;
typedef int Square;

enum { WHITE, BLACK };
enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_NONE = -1 };
enum { MOVE_CASTLE_NONE, MOVE_CASTLE_KINGSIDE, MOVE_CASTLE_QUEENSIDE };

enum {
    CASTLE_WK = 1,
    CASTLE_WQ = 2,
    CASTLE_BK = 4,
    CASTLE_BQ = 8,
    CASTLE_ALL = 15
};

#define FEN_MAX     128
#define LAN_MAX     6
#define MOVES_MAX   256
#define RENDER_MAX  2048

static inline void clear_lsb(Bitboard *bb) { *bb &= *bb - 1; }

#define rank_of(sq)   ((sq) >> 3)
#define file_of(sq)   ((sq) & 7)
#define square(r, f)  (((r) << 3) | (f))
#define bit(sq)       (1ULL << (sq))

enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQ_NONE = -1
};

typedef struct {
    Bitboard pieces[2][6];
    Bitboard occupied[2];
    Bitboard all;
    int side;
    int castling;
    int ep_square;
    int halfmove;
    int fullmove;
} Position;

typedef struct {
    uint8_t from;
    uint8_t to;
    int piece;
    int captured;
    int promotion;
    int castling;
    bool en_passant;
} Move;

typedef enum {
    STATUS_NORMAL,
    STATUS_CHECK,
    STATUS_CHECKMATE,
    STATUS_STALEMATE,
    STATUS_DRAW_50_MOVE,
    STATUS_DRAW_INSUFFICIENT,
} GameStatus;

// bitboard.c
void bitboard_init(void);
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];
extern Bitboard pawn_attacks[2][64];
extern Bitboard pawn_pushes[2][64];
Bitboard rook_attacks(Square sq, Bitboard occupied);
Bitboard bishop_attacks(Square sq, Bitboard occupied);
Bitboard queen_attacks(Square sq, Bitboard occupied);

// position.c
void position_init(Position *pos);
bool position_from_fen(Position *pos, const char *fen);
int  position_to_fen(const Position *pos, char *buf, int bufsize);
bool make_move(Position *pos, const Move *move);
int  piece_at(const Position *pos, Square sq);
int  color_at(const Position *pos, Square sq);

// movegen.c
bool is_square_attacked(const Position *pos, Square sq, int by_color);
int  generate_legal_moves(const Position *pos, Move *moves);

// notation.c
bool parse_lan(const Position *pos, const char *lan, Move *out);
int  format_lan(const Move *move, char *buf, int bufsize);

// rules.c
GameStatus get_status(const Position *pos);

// render.c
int render_board(const Position *pos, char *buf, int bufsize);

// shared utility
const char *status_name(GameStatus s);

#endif
