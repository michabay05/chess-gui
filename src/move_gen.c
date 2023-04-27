#include "move_gen.h"
#include "precalculate.h"

void movelist_add(MoveList *ml, Move move) {
  ml->list[ml->count] = move;
  ml->count++;
}

Move movelist_search(const MoveList ml, Sq source, Sq target, Piece promoted) {
  for (int i = 0; i < ml.count; i++) {
    // Parse move info
    Sq listMoveSource = move_get_source(ml.list[i]);
    Sq listMoveTarget = move_get_target(ml.list[i]);
    Piece listMovePromoted = move_get_promoted(ml.list[i]);
    // Check if source and target match
    if (listMoveSource == source && listMoveTarget == target &&
        listMovePromoted == promoted)
      // Return index of move from movelist, if true
      return ml.list[i];
  }
  return 0;
}

void movelist_print_list(const MoveList ml) {
  printf("    Source   |   Target  |  Piece  |  Promoted  |  Capture  |  Two "
         "Square Push  |  Enpassant  |  Castling\n");
  printf("  "
         "---------------------------------------------------------------------"
         "--------------------------------------\n");
  for (int i = 0; i < ml.count; i++) {
    printf("       %s    |    %s     |    %c    |     %c      |     %d     |   "
           "      %d         |      %d      |     %d\n",
           str_coords[move_get_source(ml.list[i])],
           str_coords[move_get_target(ml.list[i])],
           piece_char[move_get_piece(ml.list[i])],
           piece_char[move_get_promoted(ml.list[i])],
           move_is_capture(ml.list[i]), move_is_two_square_push(ml.list[i]),
           move_is_enpassant(ml.list[i]), move_is_castling(ml.list[i]));
  }
  printf("\n    Total number of moves: %d\n", ml.count);
}

static void movelist_gen_pawn(MoveList *ml, const Board *const b) {
  uint64_t bitboard_copy, attack_copy;
  int promotionStart, direction, doublePushStart, piece;
  int source, target;
  // If side to move is white
  if (b->state.side == LIGHT) {
    piece = lP;
    promotionStart = a7;
    direction = SOUTH;
    doublePushStart = a2;
  }
  // If side to move is black
  else {
    piece = dP;
    promotionStart = a2;
    direction = NORTH;
    doublePushStart = a7;
  }

  bitboard_copy = b->pos.piece[piece];

  while (bitboard_copy) {
    source = bb_lsb_index(bitboard_copy);
    target = source + direction;
    if ((b->state.side == LIGHT ? target >= a8 : target <= h1) &&
        !get_bit(b->pos.units[BOTH], target)) {
      // Quiet moves
      // Promotion
      if ((source >= promotionStart) && (source <= promotionStart + 7)) {
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lQ : dQ), 0, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lR : dR), 0, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lB : dB), 0, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lN : dN), 0, 0,
                                     0, 0));
      } else {
        movelist_add(ml, move_encode(source, target, piece, E, 0, 0, 0, 0));
        if ((source >= doublePushStart && source <= doublePushStart + 7) &&
            !get_bit(b->pos.units[BOTH], target + direction))
          movelist_add(ml, move_encode(source, target + direction, piece, E, 0,
                                       1, 0, 0));
      }
    }
    // Capture moves
    attack_copy =
        pawn_attacks[b->state.side][source] & b->pos.units[b->state.side ^ 1];
    while (attack_copy) {
      target = bb_lsb_index(attack_copy);
      // Capture move
      if ((source >= promotionStart) && (source <= promotionStart + 7)) {
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lQ : dQ), 1, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lR : dR), 1, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lB : dB), 1, 0,
                                     0, 0));
        movelist_add(ml, move_encode(source, target, piece,
                                     (b->state.side == LIGHT ? lN : dN), 1, 0,
                                     0, 0));
      } else
        movelist_add(ml, move_encode(source, target, piece, E, 1, 0, 0, 0));
      // Remove 'source' bit
      pop_bit(attack_copy, target);
    }
    // Generate enpassant capture
    if (b->state.enpassant != noSq) {
      uint64_t enpassCapture =
          pawn_attacks[b->state.side][source] & (1ULL << b->state.enpassant);
      if (enpassCapture) {
        int enpassTarget = bb_lsb_index(enpassCapture);
        movelist_add(ml,
                     move_encode(source, enpassTarget, piece, E, 1, 0, 1, 0));
      }
    }
    // Remove bits
    pop_bit(bitboard_copy, source);
  }
}

static void movelist_gen_knight(MoveList *ml, const Board *const b) {
  int source, target, piece = b->state.side == LIGHT ? lN : dN;
  uint64_t bitboard_copy = b->pos.piece[piece], attack_copy;
  while (bitboard_copy) {
    source = bb_lsb_index(bitboard_copy);

    attack_copy =
        knight_attacks[source] &
        (b->state.side == LIGHT ? ~b->pos.units[LIGHT] : ~b->pos.units[DARK]);
    while (attack_copy) {
      target = bb_lsb_index(attack_copy);
      if (get_bit(b->pos.units[b->state.side == LIGHT ? DARK : LIGHT], target))
        movelist_add(ml, move_encode(source, target, piece, E, 1, 0, 0, 0));
      else
        movelist_add(ml, move_encode(source, target, piece, E, 0, 0, 0, 0));
      pop_bit(attack_copy, target);
    }
    pop_bit(bitboard_copy, source);
  }
}

static void movelist_gen_bishop(MoveList *ml, const Board *const b) {
  int source, target, piece = b->state.side == LIGHT ? lB : dB;
  uint64_t bitboard_copy = b->pos.piece[piece], attack_copy;
  while (bitboard_copy) {
    source = bb_lsb_index(bitboard_copy);

    attack_copy =
        get_bishop_attack(source, b->pos.units[BOTH]) &
        (b->state.side == LIGHT ? ~b->pos.units[LIGHT] : ~b->pos.units[DARK]);

    bb_print(get_bishop_attack(source, b->pos.units[BOTH]));
    bb_print(~b->pos.units[DARK]);

    while (attack_copy) {
      target = bb_lsb_index(attack_copy);
      if (get_bit(b->pos.units[b->state.side == LIGHT ? DARK : LIGHT], target))
        movelist_add(ml, move_encode(source, target, piece, E, 1, 0, 0, 0));
      else
        movelist_add(ml, move_encode(source, target, piece, E, 0, 0, 0, 0));
      pop_bit(attack_copy, target);
    }
    pop_bit(bitboard_copy, source);
  }
}

static void movelist_gen_rook(MoveList *ml, const Board *const b) {
  int source, target, piece = b->state.side == LIGHT ? lR : dR;
  uint64_t bitboard_copy = b->pos.piece[piece], attack_copy;
  while (bitboard_copy) {
    source = bb_lsb_index(bitboard_copy);

    attack_copy =
        get_rook_attack(source, b->pos.units[BOTH]) &
        (b->state.side == LIGHT ? ~b->pos.units[LIGHT] : ~b->pos.units[DARK]);
    while (attack_copy) {
      target = bb_lsb_index(attack_copy);
      if (get_bit(b->pos.units[b->state.side == LIGHT ? DARK : LIGHT], target))
        movelist_add(ml, move_encode(source, target, piece, E, 1, 0, 0, 0));
      else
        movelist_add(ml, move_encode(source, target, piece, E, 0, 0, 0, 0));
      pop_bit(attack_copy, target);
    }
    pop_bit(bitboard_copy, source);
  }
}

static void movelist_gen_queen(MoveList *ml, const Board *const b) {
  int source, target, piece = b->state.side == LIGHT ? lQ : dQ;
  uint64_t bitboard_copy = b->pos.piece[piece], attack_copy;
  while (bitboard_copy) {
    source = bb_lsb_index(bitboard_copy);

    attack_copy =
        get_queen_attack(source, b->pos.units[BOTH]) &
        (b->state.side == LIGHT ? ~b->pos.units[LIGHT] : ~b->pos.units[DARK]);
    while (attack_copy) {
      target = bb_lsb_index(attack_copy);
      if (get_bit(b->pos.units[b->state.side == LIGHT ? DARK : LIGHT], target))
        movelist_add(ml, move_encode(source, target, piece, E, 1, 0, 0, 0));
      else
        movelist_add(ml, move_encode(source, target, piece, E, 0, 0, 0, 0));
      pop_bit(attack_copy, target);
    }
    pop_bit(bitboard_copy, source);
  }
}

void movelist_generate(MoveList *ml, const Board *const b) {
  movelist_gen_pawn(ml, b);
  movelist_gen_knight(ml, b);
  movelist_gen_bishop(ml, b);
  movelist_gen_rook(ml, b);
  movelist_gen_queen(ml, b);
}
