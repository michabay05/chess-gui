#include "gui_board.h"
#include "board.h"
#include "defs.h"
#include "move.h"
#include "precalculate.h"

static bool is_in_bound(Vector2 pos, Rectangle rec) {
  return (pos.x >= rec.x && pos.x <= rec.x + rec.width) &&
         (pos.y >= rec.y && pos.y <= rec.y + rec.height);
}

static void gui_set_selected(GUI_Board *gb) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouse_pos = GetMousePosition();
    if (is_in_bound(mouse_pos, gb->boundary)) {
      mouse_pos.x -= padding[0];
      mouse_pos.y -= padding[1];
      gb->selected = SQ(mouse_pos.y / SQ_SIZE, mouse_pos.x / SQ_SIZE);
      return;
    }
    gb->selected = noSq;
  }
}

static void gui_update_preview(GUI_Board *gb) {
  gb->preview = 0;
  if (gb->selected == noSq) return;
  int piece = pos_get_piece(gb->board.pos, gb->selected);
  if (piece == E) return;
  uint64_t attack_moves = 0;
  switch (COLORLESS(piece)) {
  case PAWN:
    attack_moves = pawn_attacks[piece < 6 ? LIGHT : DARK][gb->selected];
    break;
  case KNIGHT:
    attack_moves = knight_attacks[gb->selected];
    break;
  case BISHOP:
    attack_moves = get_bishop_attack(gb->selected, gb->board.pos.units[BOTH]);
    break;
  case ROOK:
    attack_moves = get_rook_attack(gb->selected, gb->board.pos.units[BOTH]);
    break;
  case QUEEN:
    attack_moves = get_queen_attack(gb->selected, gb->board.pos.units[BOTH]);
    break;
  case KING:
    attack_moves = king_attacks[gb->selected];
    break;
  }
  gb->preview = attack_moves;
}

void gui_board_init(GUI_Board *gb) {
  gb->boundary = (Rectangle){padding[0], padding[1], 8 * SQ_SIZE, 8 * SQ_SIZE};
  gb->board = (Board){0};
  gb->selected = noSq;
}

void gui_board_update(GUI_Board *gb) {
  gui_set_selected(gb);
  gui_update_preview(gb);
}
