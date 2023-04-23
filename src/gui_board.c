#include "gui_board.h"
#include "move.h"
#include <raylib.h>
#include <string.h>

static bool is_in_bound(Vector2 pos, Rectangle rec) {
  return (pos.x >= rec.x && pos.x <= rec.x + rec.width) &&
         (pos.y >= rec.y && pos.y <= rec.y + rec.height);
}

static void gui_set_selected(GUI_Board* gb) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Vector2 mouse_pos = GetMousePosition();
		if (is_in_bound(mouse_pos, gb->boundary)) {
			mouse_pos.x -= padding[0];
			mouse_pos.y -= padding[1];
			gb->selected = SQ(mouse_pos.y / SQ_SIZE, mouse_pos.x / SQ_SIZE);
		}
	}
}

static void gui_update_preview(GUI_Board* gb) {
	gb->preview = 0;
	if (gb->selected != a2) return;
	set_bit(gb->preview, a8);
	set_bit(gb->preview, c2);
	set_bit(gb->preview, d6);
	set_bit(gb->preview, f5);
}

void gui_board_init(GUI_Board* gb) {
	gb->boundary = (Rectangle) {padding[0], padding[1], 8 * SQ_SIZE, 8 * SQ_SIZE};
	gb->board = (Board){0};
	gb->selected = noSq;
}

void gui_board_update(GUI_Board* gb) {
	gui_set_selected(gb);
	gui_update_preview(gb);
}
