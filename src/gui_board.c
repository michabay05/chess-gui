#include "board.h"
#include "defs.h"
#include "move.h"
#include "precalculate.h"
#include "gui_defs.h"

static Sq coord_to_sq(Vector2 mouse_pos, Rectangle sec) {
    if (CheckCollisionPointRec(mouse_pos, sec)) {
        mouse_pos.x -= sec.x;
        mouse_pos.y -= sec.y;
        return SQ(
            (int)(mouse_pos.y / (sec.height / 8.f)),
            (int)(mouse_pos.x / (sec.width / 8.f))
        );
    }
    return noSq;
}

static void gui_board_set_selected(GUI_Board *gb, Rectangle sec) {
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

static void gui_board_set_target(GUI_Board *gb, Rectangle sec) {
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

bool is_draw_by_insufficient_material(GUI_Board* gb)
{
    (void) gb;
    // TODO: implement this function

    // List of endgames drawn due to insufficient material
    // -> K  vs k
    // -> KN vs kn
    // -> K  vs kn
    // -> K  vs k
    // -> KB vs kb
    // -> KB vs k
    // -> K  vs kb

    return false;
}

void update_game_state(GUI_Board* gb)
{
    MoveList legal = { 0 };
    movelist_legal(&legal, &gb->board);
    if (legal.count == 0) {
        PieceColor c = gb->board.state.side;
        gb->board.state.side = c ^ 1;
        if (board_is_in_check(&gb->board))
            gb->state = GS_CHECKMATE;
        else
            gb->state = GS_DRAW_STALEMATE;
        gb->board.state.side = c;
    } else if (is_draw_by_insufficient_material(gb)) {
        gb->state = GS_DRAW_INSUFF_MAT;
    } else {
        gb->state = GS_ONGOING;
    }
}

void gui_board_init(GUI_Board *gb, const char* fen_str) {
    gb->board = (Board){0};
    FENInfo fen = parse_fen(fen_str);
    board_set_from_fen(&gb->board, fen);

    gb->is_promotion = false;
    gb->promoted_choice = E;
    gb->selected = noSq;
    gb->target = noSq;
    gb->preview = 0ULL;
    update_game_state(gb);
}

void gui_board_update(GUI_Board *gb, FILE* record_fptr, Rectangle sec) {
    gui_board_set_selected(gb, sec);
    gui_board_update_preview(gb);
    gui_board_set_target(gb, sec);
    gui_board_make_move(gb, record_fptr);
    update_game_state(gb);
}
