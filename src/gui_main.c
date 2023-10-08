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

void draw_piece(Texture2D *tex, Piece piece, int row, int col, Section sec)
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

void draw_promotion_options(Texture* tex, bool is_white, Section sec)
{
    Vector2 rect_size = { sec.size.x * 0.6f, sec.size.y * 0.15f };
    Vector2 center = {
        (sec.padding.x + sec.size.x / 2.f) - (rect_size.x / 2.f),
        (sec.padding.y + sec.size.y / 2.f) - (rect_size.y / 2.f)
    };
    DrawRectangleV(
        center,
        rect_size,
        (Color) { 20, 20, 20, 150 }
    );

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    for (int i = 0; i < 4; i++) {
        float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
        float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
        int color = is_white;
        int type = i + 1;

        const float SCALE = 0.85; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
        const float SQ_SIZE = rect_size.x * 0.25f;
        float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
        float pos[2] = { center.x + i * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f), center.y };
        /* DrawRectangleV(
            (Vector2) { center.x + i * SQ_SIZE, center.y },
            (Vector2) { SQ_SIZE, rect_size.y },
            (Color) { 200, 0, 0, 50 }
        ); */
        DrawTexturePro(*tex,
                    (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                    (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
    }
}

void choose_promotion_piece(GUI_Board* gb, Section sec)
{
    Vector2 rect_size = { sec.size.x * 0.6f, sec.size.y * 0.15f };
    Vector2 center = {
        (sec.padding.x + sec.size.x / 2.f) - (rect_size.x / 2.f),
        (sec.padding.y + sec.size.y / 2.f) - (rect_size.y / 2.f)
    };
    Vector2 mouse_pos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse_pos, (Rectangle){center.x, center.y, rect_size.x, rect_size.y})) {
        mouse_pos = Vector2Subtract(mouse_pos, center); 
        gb->promoted_choice = (int)(mouse_pos.x / (rect_size.x / 4.f)) + 1;
        if (gb->board.state.side == DARK) gb->promoted_choice += 6;
        gb->is_promotion = false;
   }
}

void draw_eval_bar(float eval, Section sec)
{
    const float MAGIC_FACTOR = 0.1443f;
    float scale = sqrt(fabsf(eval)) * MAGIC_FACTOR;
    scale *= eval < 0 ? 1 : -1;
    scale += 0.5;

    DrawRectangleV(
        sec.padding,
        (Vector2) { sec.size.x, sec.size.y * scale },
        (Color) { 15, 15, 15, 255 }
    );

    DrawRectangleV(
        (Vector2) { sec.padding.x, sec.padding.y + sec.size.y * scale },
        (Vector2) { sec.size.x, sec.size.y * (1 - scale) },
        RAYWHITE
    );

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
    
#if 1
    FENInfo fen = parse_fen("8/8/1k6/8/p3K3/8/8/8 w - - 0 1");
#else
    FENInfo fen = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
#endif
    UI_State us = { 0 };
    GUI_Board gb = { 0 };
    gui_board_init(&gb, "moves.txt");
    board_set_from_fen(&gb.board, fen);

    us.font =  LoadFontEx("assets/font/Rubik-Regular.ttf", 30, NULL, 0);

    Vector2 current_pos = { 0 };
    float eval = 0;

    while (!WindowShouldClose()) {
        float horz_padding = GetRenderWidth() * 0.01f;
        float vert_padding = GetRenderHeight() * 0.03f;

        float eval_vert_size = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        float eval_vert_pad = fmax(GetRenderWidth() - eval_vert_size, GetRenderHeight() - eval_vert_size);
        us.sections[SECTION_EVAL_BAR] = (Section) {
            .size = (Vector2) { GetRenderWidth() * 0.03f, eval_vert_size},
            .padding = (Vector2) {horz_padding, vert_padding}
        };
        current_pos.x = us.sections[SECTION_EVAL_BAR].padding.x + us.sections[SECTION_EVAL_BAR].size.x;
        current_pos.y = us.sections[SECTION_EVAL_BAR].padding.y;

        float squares_area = fmin(0.6 * GetRenderWidth(), 0.95 * GetRenderHeight());
        us.sections[SECTION_BOARD] = (Section) {
            .size = (Vector2) { squares_area, squares_area },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y,
            }
        };
        current_pos.x = us.sections[SECTION_BOARD].padding.x + us.sections[SECTION_BOARD].size.x;
        current_pos.y = us.sections[SECTION_BOARD].padding.y;

        us.sections[SECTION_MOVES_LIST] = (Section) {
            .size = (Vector2) { GetRenderWidth() - (current_pos.x + 2 * horz_padding), GetRenderHeight() * 0.6f },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y
            },
        };
        current_pos.y = us.sections[SECTION_MOVES_LIST].padding.y + us.sections[SECTION_MOVES_LIST].size.y;

        us.sections[SECTION_INFO] = (Section) {
            .size = (Vector2) {
                GetRenderWidth() - (current_pos.x + 2 * horz_padding),
                GetRenderHeight() - (current_pos.y + 2 * vert_padding)
            },
            .padding = (Vector2) {
                current_pos.x + horz_padding,
                current_pos.y + vert_padding
            },
        };

        if (gb.is_promotion)
            choose_promotion_piece(&gb, us.sections[SECTION_BOARD]);

        gui_board_update(&gb, us.sections[SECTION_BOARD]);
        BeginDrawing();
        {
            ClearBackground(BACKGROUND);

            draw_eval_bar(eval, us.sections[SECTION_EVAL_BAR]);
            draw_board(gb, us.sections[SECTION_BOARD]);
            for (int i = 0; i < 64; i++) {
                draw_piece(&tex, pos_get_piece(gb.board.pos, i), ROW(i), COL(i), us.sections[SECTION_BOARD]);
            }
            if (gb.is_promotion)
                draw_promotion_options(&tex, gb.board.state.side, us.sections[SECTION_BOARD]);
            draw_moves_list(us.sections[SECTION_MOVES_LIST]);
            draw_info(us.sections[SECTION_INFO]);
            
            DrawTextEx(us.font, TextFormat("Eval: %.1f", eval),
                       (Vector2) { 700, 500 }, us.font.baseSize, 0, WHITE); // Draw text using font and additional parameters
        }
        EndDrawing();
    }

    // De-initialization
    fclose(gb.fptr);
    UnloadFont(us.font);
    UnloadTexture(tex);
    CloseWindow();

    return 0;
}
