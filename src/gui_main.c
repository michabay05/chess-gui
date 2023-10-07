#include "board.h"
#include "fen.h"
#include "gui_defs.h"

#include "raymath.h"

void draw_board(const GUI_Board gb, Section sec)
{
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            Color sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
            if ((int)gb.selected == SQ(r, f) && pos_get_piece(gb.board.pos, gb.selected) != E)
                sq_clr = ColorAlphaBlend(sq_clr, SELECTED_COLOR, WHITE);
            if (get_bit(gb.preview, SQ(r, f)))
                sq_clr = ColorAlphaBlend(sq_clr, PREVIEW_COLOR, WHITE);

            DrawRectangleV(
                (Vector2) {sec.padding.x + f * sec.size.x / 8.f, sec.padding.y + r * sec.size.y / 8.f }, 
                (Vector2) {sec.size.x / 8.f, sec.size.y / 8.f},
                sq_clr
            );
        }
    }
    DrawRectangleLines(sec.padding.x, sec.padding.y, sec.size.x, sec.size.y, RED);
}

void draw_pieces(Texture2D *tex, Piece piece, int row, int col, Section sec)
{
    if (piece == E) return;

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
    float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
    int color = piece >= 6;
    int type = piece;

    const float SCALE = 0.95; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
    const float SQ_SIZE = sec.size.x / 8.f;
    float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
    float pos[2] = {
        sec.padding.x + col * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f),
        sec.padding.y + row * SQ_SIZE + ((SQ_SIZE - texture_dim[1]) / 2.0f),
    };
    DrawTexturePro(*tex,
                   (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                   (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
}

void draw_promotion_options(Section sec)
{
    Vector2 rect_size = { sec.size.x * 0.6f, sec.size.y * 0.125f };
    Vector2 center = {
        (sec.padding.x + sec.size.x / 2.f) - (rect_size.x / 2.f),
        (sec.padding.y + sec.size.y / 2.f) - (rect_size.y / 2.f)
    };
    /* DrawRectangleV(
        center,
        rect_size,
        (Color) { 20, 20, 20, 150 }
    ); */

    for (int i = 0; i < 4; i++) {
        DrawRectangleV(
            (Vector2) { center.x + 2.5 + i * (rect_size.x * 0.25f), center.y },
            (Vector2) { rect_size.x * 0.25f - 5, rect_size.y },
            (Color) { 200, 0, 0, 50 }
        );
    }
}

void draw_eval_bar(Section sec)
{
    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        BLUE
    );
}

void draw_moves_list(Section sec)
{
    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        GREEN
    );
}

void draw_info(Section sec)
{
    DrawRectangleLines(
        sec.padding.x,
        sec.padding.y,
        sec.size.x,
        sec.size.y,
        WHITE
    );
}

int gui_main(void)
{
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");
    SetExitKey(KEY_Q);
    SetTargetFPS(30);

    Texture2D tex = LoadTexture("assets/neo-pieces-spritesheet.png");
    FENInfo fen = parse_fen(
        "6R1/P2k4/r7/5N1P/r7/p7/7K/8 w - - 0 1");
        // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    UI_State state = { 0 };
    GUI_Board gb = { 0 };
    gui_board_init(&gb);
    board_set_from_fen(&gb.board, fen);

    Vector2 current_pos = { 0 };

    while (!WindowShouldClose()) {
        float horz_padding = GetRenderWidth() * 0.01f;
        float vert_padding = GetRenderHeight() * 0.03f;

        float eval_vert_size = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        float eval_vert_pad = fmax(GetRenderWidth() - eval_vert_size, GetRenderHeight() - eval_vert_size);
        state.sections[SECTION_EVAL_BAR] = (Section) {
            .size = (Vector2) { GetRenderWidth() * 0.03f, eval_vert_size},
            .padding = (Vector2) {horz_padding, vert_padding}
        };
        current_pos.x = state.sections[SECTION_EVAL_BAR].padding.x + state.sections[SECTION_EVAL_BAR].size.x;
        current_pos.y = state.sections[SECTION_EVAL_BAR].padding.y;

        float squares_area = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        state.sections[SECTION_BOARD] = (Section) {
            .size = (Vector2) { squares_area, squares_area },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y,
            }
        };
        current_pos.x = state.sections[SECTION_BOARD].padding.x + state.sections[SECTION_BOARD].size.x;
        current_pos.y = state.sections[SECTION_BOARD].padding.y;

        state.sections[SECTION_MOVES_LIST] = (Section) {
            .size = (Vector2) { GetRenderWidth() - (current_pos.x + 2 * horz_padding), GetRenderHeight() * 0.6f },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y
            },
        };
        current_pos.y = state.sections[SECTION_MOVES_LIST].padding.y + state.sections[SECTION_MOVES_LIST].size.y;

        state.sections[SECTION_INFO] = (Section) {
            .size = (Vector2) {
                GetRenderWidth() - (current_pos.x + 2 * horz_padding),
                GetRenderHeight() - (current_pos.y + 2 * vert_padding)
            },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y + vert_padding
            },
        };

        gui_board_update(&gb, state.sections[SECTION_BOARD]);
        BeginDrawing();
        {
            ClearBackground(BACKGROUND);

            draw_eval_bar(state.sections[SECTION_EVAL_BAR]);
            draw_board(gb, state.sections[SECTION_BOARD]);
            for (int i = 0; i < 64; i++) {
                draw_pieces(&tex, pos_get_piece(gb.board.pos, i), ROW(i), COL(i), state.sections[SECTION_BOARD]);
            }
            draw_promotion_options(state.sections[SECTION_BOARD]);
            draw_moves_list(state.sections[SECTION_MOVES_LIST]);
            draw_info(state.sections[SECTION_INFO]);
        }
        EndDrawing();
    }
    UnloadTexture(tex);
    CloseWindow();

    return 0;
}
