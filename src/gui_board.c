#include "board.h"
#include "defs.h"
#include "move.h"
#include "precalculate.h"
#include "gui_defs.h"

static Sq coord_to_sq(Vector2 mouse_pos, Section sec) {
    if (CheckCollisionPointRec(mouse_pos, (Rectangle){sec.padding.x, sec.padding.y, sec.size.x, sec.size.y})) {
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
        gb->target = temp_selected;
        if (pos_get_piece(gb->board.pos, gb->selected) % 6 == 0 &&
            (ROW(gb->target) == 0 || ROW(gb->target) == 7)) {
            gb->is_promotion = true;
        }
    }

}

static void record_move(FILE* fptr, int move_count, bool is_white, Move move)
{
    if (fptr == NULL) return;

    // @FIX BUG: the '1. ..' doesn't work when white moves first
    // Example problem: 
    //             v---v
        // 1. e4d4 1. .. a4a3 
        // 2. d4c3  a3a2 
    // I might also need to refactor the move counting system
    if (move_count == 1 && !is_white) {
        fprintf(fptr, "1. ..");
    } else if (is_white) {
        fprintf(fptr, "%d.", move_count);
    }
    
    char move_str[6] = { 0 };
    move_to_str(move, move_str);
    fprintf(fptr, " %s", move_str);
    if (!is_white)
        fprintf(fptr, "\n");
}

static void gui_board_make_move(GUI_Board* gb, FILE* record_fptr)
{
    if (gb->target == noSq) return;
    if (gb->is_promotion) return;

    Move mv = movelist_search(gb->ml, gb->selected, gb->target, gb->promoted_choice);
    if (mv == E) return;
    int move_count = gb->board.state.full_moves;
    if (!move_make(&gb->board, mv, AllMoves)) {
        TraceLog(LOG_ERROR, "Failed to make move");
    }
    record_move(record_fptr, move_count, gb->board.state.side == DARK, mv);

    gb->selected = noSq;
    gb->target = noSq;
    gb->promoted_choice = E;
    gb->is_promotion = false;
}

static void gui_board_update_preview(GUI_Board* gb)
{
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
}

void gui_board_init(GUI_Board *gb) {
    gb->board = (Board){0};
    gb->is_promotion = false;
    gb->promoted_choice = E;
    gb->selected = noSq;
    gb->target = noSq;
    gb->preview = 0ULL;
}

void gui_board_update(GUI_Board *gb, FILE* record_fptr, Section sec) {
    gui_board_set_selected(gb, sec);
    gui_board_update_preview(gb);
    gui_board_set_target(gb, sec);
    gui_board_make_move(gb, record_fptr);
}
