#include "gui_board.h"
#include "board.h"
#include "defs.h"
#include "move.h"
#include "precalculate.h"

static bool is_in_bound(Vector2 pos, Rectangle rec) {
    return (pos.x >= rec.x && pos.x <= rec.x + rec.width) &&
           (pos.y >= rec.y && pos.y <= rec.y + rec.height);
}

static Sq coord_to_sq(Vector2 mouse_pos, Rectangle bounds) {
    if (is_in_bound(mouse_pos, bounds)) {
        mouse_pos.x -= padding[0];
        mouse_pos.y -= padding[1];
        return SQ(mouse_pos.y / SQ_SIZE, mouse_pos.x / SQ_SIZE);
    }
    return noSq;
}

static void gui_set_selected(GUI_Board *gb) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Sq temp_selected = coord_to_sq(GetMousePosition(), gb->boundary);
        Sq prev_selected = gb->selected;
        if (temp_selected == noSq) return;
        if (pos_get_piece(gb->board.pos, temp_selected) == E) return;
        if (prev_selected == temp_selected) {
            gb->selected = noSq;
            return;
        }
        gb->selected = temp_selected;
    }
}

static void gui_set_target(GUI_Board *gb) {
    if (gb->selected == noSq) return;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Sq temp_selected = coord_to_sq(GetMousePosition(), gb->boundary);
        if (temp_selected == gb->selected) return;
        if (!get_bit(gb->preview, temp_selected)) return;
        gb->target = temp_selected;
        printf("Selected: %s\n  Target: %s\n\n", str_coords[gb->selected], str_coords[gb->target]);
    }
}

#include "move.h"
#include "move_gen.h"
static void gui_update_preview(GUI_Board *gb) {
    gb->preview = 0;
    if (gb->selected == noSq)
        return;
    Piece piece = pos_get_piece(gb->board.pos, gb->selected);
    if (piece == E)
        return;

    MoveList ml = {0};
    movelist_generate(&ml, &gb->board, piece);
    for (int i = 0; i < ml.count; i++) {
        // If the current move's source square is the selected square
        // Turn on the target bit of the move on the preview bitboard
        if (move_get_source(ml.list[i]) != gb->selected)
            continue;
        set_bit(gb->preview, move_get_target(ml.list[i]));
    }
}

void gui_board_init(GUI_Board *gb) {
    gb->boundary = (Rectangle){padding[0], padding[1], 8 * SQ_SIZE, 8 * SQ_SIZE};
    gb->board = (Board){0};
    gb->selected = noSq;
}

void gui_board_update(GUI_Board *gb) {
    gui_set_selected(gb);
    gui_update_preview(gb);
    gui_set_target(gb);
}
