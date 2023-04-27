#include "board.h"
#include "defs.h"
#include <string.h>

const char* str_coords[65] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7",
    "d7", "e7", "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6",
    "g6", "h6", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a4",
    "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a3", "b3", "c3", "d3",
    "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", " "};

const char piece_char[13] = {
  'P', 'N', 'B', 'R', 'Q', 'K',
  'p', 'n', 'b', 'r', 'q', 'k', ' '
};

void pos_add_piece(Position *pos, Piece piece, Sq sq) {
  set_bit(pos->piece[piece], sq);
  pos_update_units(pos);
}

void pos_remove_piece(Position *pos, Piece piece, Sq sq) {
  pop_bit(pos->piece[piece], sq);
  pos_update_units(pos);
}

Piece pos_get_piece(const Position pos, Sq sq) {
  for (int i = lP; i <= dK; i++) {
    if (get_bit(pos.piece[i], sq)) {
      return i;
    }
  }
  return E;
}

void pos_update_units(Position *pos) {
  // Reset all the units bitboard to 0
  memset(pos->units, 0, sizeof(pos->units));
  for (int i = PAWN; i <= KING; i++) {
    pos->units[LIGHT] |= pos->piece[i];
    pos->units[DARK] |= pos->piece[6 + i];
  }
  pos->units[BOTH] = pos->units[LIGHT] | pos->units[DARK];
}

void change_side(State *state) { state->side = !state->side; }

void board_set_from_fen(Board *board, FENInfo fen) {
  *board = (Board){0};
  // Set pieces
  for (int i = 0; i < 64; i++) {
    if (fen.board[i] == E)
      continue;
    pos_add_piece(&board->pos, fen.board[i], i);
  }
  board->state.side = fen.side;
  board->state.castling = fen.castling;
  board->state.enpassant = fen.enpassant;
  board->state.half_moves = fen.half_moves;
  board->state.full_moves = fen.full_moves;
}

typedef enum {
  lk,
  lq,
  dk,
  dq
} CastlingRight;

static void print_castling(int castling) {
  char castling_str[4] = { '-', '-', '-', '-' };
  if (castling & lk)
    castling_str[0] = 'K';
  if (castling & lq)
    castling_str[1] = 'Q';
  if (castling & dk)
    castling_str[2] = 'k';
  if (castling & dk)
    castling_str[3] = 'q';
  printf("%s\n", castling_str);
}

void board_print(const Board* const b) {
    printf("\n    +---+---+---+---+---+---+---+---+\n");
  for (int r = 0; r < 8; r++) {
    printf("  %d |", 8 - r);
    for (int f = 0; f < 8; f++) {
      printf(" %c |", piece_char[pos_get_piece(b->pos, SQ(r,f))]);
    }
    printf("\n    +---+---+---+---+---+---+---+---+\n");
  }
  printf("      a   b   c   d   e   f   g   h\n\n");
  printf("        Side: %s\n", !b->state.side ? "white" : "black");
  printf("    Castling: ");
  print_castling(b->state.castling);
  printf("       Moves: %d\n", b->state.full_moves);
}
