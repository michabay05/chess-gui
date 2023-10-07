#include "board.h"
#include "defs.h"
#include "move.h"
#include "precalculate.h"
#include "gui_defs.h"

static bool is_in_bound(Vector2 pos, Section sec) {
    return (pos.x >= sec.padding.x && pos.x <= sec.padding.x + sec.size.x) &&
           (pos.y >= sec.padding.y && pos.y <= sec.padding.y + sec.size.y);
}

static Sq coord_to_sq(Vector2 mouse_pos, Section sec) {
    if (is_in_bound(mouse_pos, sec)) {
        mouse_pos.x -= sec.padding.x;
        mouse_pos.y -= sec.padding.y;
        return SQ(
            (int)(mouse_pos.y / (sec.size.y / 8.f)),
            (int)(mouse_pos.x / (sec.size.x / 8.f))
        );
    }
    return noSq;
}

static void gui_board_set_selected(GUI_Board *gb, Section sec) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (gb->selected != noSq) return;
        Sq temp_selected = coord_to_sq(GetMousePosition(), sec);
        Sq prev_selected = gb->selected;
        if (temp_selected == noSq) return;
        if (pos_get_piece(gb->board.pos, temp_selected) == E) return;
        if (prev_selected == temp_selected) {
            gb->selected = noSq;
            return;
        }
        gb->selected = temp_selected;
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        gb->selected = noSq;
    }
}

static void gui_board_set_target(GUI_Board *gb, Section sec) {
    if (gb->selected == noSq) return;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Sq temp_selected = coord_to_sq(GetMousePosition(), sec);
        if (temp_selected == gb->selected) return;
        if (!get_bit(gb->preview, temp_selected)) return;
        bool is_capture = false;
        if (pos_get_piece(gb->board.pos, temp_selected) != E) {
            is_capture = true;
        }
        gb->target = temp_selected;
        Move mv = movelist_search(gb->ml, gb->selected, gb->target, E);
        if (mv == E) return;
        if (!move_make(&gb->board, mv, AllMoves)) {
            TraceLog(LOG_ERROR, "Failed to make move");
        }
        gb->selected = noSq;
        gb->target = noSq;
    }
}

static void gui_board_update_preview(GUI_Board *gb) {
    gb->preview = 0;
    if (gb->selected == noSq)
        return;
    Piece piece = pos_get_piece(gb->board.pos, gb->selected);
    if (piece == E)
        return;

    gb->ml = (MoveList){0};
    movelist_generate(&gb->ml, &gb->board, piece);
    Board clone;
    for (int i = 0; i < gb->ml.count; i++) {
        clone = gb->board;
        // If the current move's source square is the selected square
        // Turn on the target bit of the move on the preview bitboard
        if (move_get_source(gb->ml.list[i]) != gb->selected)
            continue;
        if (move_make(&gb->board, gb->ml.list[i], AllMoves)) {
            set_bit(gb->preview, move_get_target(gb->ml.list[i]));
        }
        gb->board = clone;
    }
    TraceLog(LOG_INFO, "Regenerated move list\n");
}

void gui_board_init(GUI_Board *gb) {
    gb->board = (Board){0};
    gb->selected = noSq;
    gb->target = noSq;
    gb->preview = 0ULL;
}

void gui_board_update(GUI_Board *gb, Section sec) {
    gui_board_set_selected(gb, sec);
    gui_board_update_preview(gb);
    gui_board_set_target(gb, sec);
}
