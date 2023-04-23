#include "move.h"
#include "defs.h"

Move move_encode(Sq source, Sq target, Piece piece, Piece promoted,
                bool isCapture, bool isTwoSquarePush, bool isEnpassant,
                bool isCastling) {
  return source | (target << 6) | (piece << 12) | (promoted << 16) |
         (isCapture << 20) | (isTwoSquarePush << 21) | (isEnpassant << 22) |
         (isCastling << 23);
}

int move_get_source(const Move move) { return move & 0x3F; }
int move_get_target(const Move move) { return (move & 0xFC0) >> 6; }
int move_get_piece(const Move move) { return (move & 0xF000) >> 12; }
int move_get_promoted(const Move move) {
  int promoted = (move & 0xF0000) >> 16;
  return promoted ? promoted : E;
}

char move_promoted_char(const Move move) {
  char promoted;
  Piece promoted_piece = move_get_promoted(move);
  switch (promoted_piece) {
  case lQ:
    promoted = 'Q';
    break;
  case lR:
    promoted = 'R';
    break;
  case lB:
    promoted = 'B';
    break;
  case lN:
    promoted = 'N';
    break;
  case dQ:
    promoted = 'q';
    break;
  case dR:
    promoted = 'r';
    break;
  case dB:
    promoted = 'b';
    break;
  case dN:
    promoted = 'n';
    break;
  default:
    promoted = ' ';
    break;
  }
  return promoted;
}

bool move_is_capture(const Move move) { return move & 0x100000; }
bool move_is_two_square_push(const Move move) { return move & 0x200000; }
bool move_is_enpassant(const Move move) { return move & 0x400000; }
bool move_is_castling(const Move move) { return move & 0x800000; }

void move_to_str(const Move move, char* move_str) {
  // Source square
  move_str[0] = str_coords[move_get_source(move)][0];
  move_str[1] = str_coords[move_get_source(move)][1];
  // Target square
  move_str[2] = str_coords[move_get_target(move)][2];
  move_str[3] = str_coords[move_get_target(move)][3];
  // Promotion piece
  move_str[4] = move_promoted_char(move);
}

Move move_parse(char *move_str, Piece piece, bool is_capture,
               bool is_two_square_push, bool is_enpassant, bool is_castling) {
  int source = SQ(move_str[1] - '0', move_str[0] - 'a');
  int target = SQ(move_str[3] - '0', move_str[2] - 'a');
  Piece promoted;
  if (move_str && (move_str[4] >= 'a' && move_str[4] <= 'z')) {
    switch (move_str[4]) {
    case 'Q':
      promoted = lQ;
      break;
    case 'R':
      promoted = lR;
      break;
    case 'B':
      promoted = lB;
      break;
    case 'N':
      promoted = lN;
      break;
    case 'q':
      promoted = dQ;
      break;
    case 'r':
      promoted = dR;
      break;
    case 'b':
      promoted = dB;
      break;
    case 'n':
      promoted = dN;
      break;
    }    
  }
  int ouptut = move_encode(source, target, piece, promoted, is_capture, is_two_square_push, is_enpassant, is_castling);

  char move_temp[5];
  move_to_str(ouptut, move_temp);

  return ouptut;
}
