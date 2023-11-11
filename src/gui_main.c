#include "fen.h"
#include "gui_defs.h"

#include "raymath.h"

#include <string.h>

void draw_board(const GUI_Board* const gb, Rectangle sec, Font font)
{
    Color sq_clr, opp_clr;
    const float SQ_SIZE = fminf(sec.width, sec.height) / 8.f;
    const float COORD_TEXT_SIZE = font.baseSize * (0.001f * sec.width);
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
            opp_clr = SQCLR(r, f) ? DARK_CLR : LIGHT_CLR;
            if ((int)gb->selected == SQ(r, f) && pos_get_piece(gb->board.pos, gb->selected) != E)
                sq_clr = ColorAlphaBlend(sq_clr, SELECTED_COLOR, WHITE);
            if (get_bit(gb->preview, SQ(r, f)))
                sq_clr = ColorAlphaBlend(sq_clr, PREVIEW_COLOR, WHITE);

            DrawRectangleV(
                (Vector2) {sec.x + f * SQ_SIZE, sec.y + r * SQ_SIZE }, 
                Vector2Scale(Vector2One(), SQ_SIZE),
                sq_clr
            );
            if (r == 7) {
                DrawTextEx(font, 
                           TextFormat("%c", 'a' + f), 
                           (Vector2) {
                           sec.x + f * SQ_SIZE + SQ_SIZE - (2.5f * 0.01f * sec.width),
                           sec.y + 0.96f * sec.height
                           },
                           COORD_TEXT_SIZE, 0, opp_clr);
            }
        }
        DrawTextEx(font, 
                   TextFormat("%d", 8 - r), 
                   (Vector2) { sec.x + 0.01 * sec.width, sec.y + r * SQ_SIZE + 0.01 * sec.height },
                   COORD_TEXT_SIZE, 0, sq_clr);
    }
}

void draw_piece(Texture2D *tex, Piece piece, int row, int col, Rectangle sec)
{
    if (piece == E) return;

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
    float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
    int color = piece >= 6;
    int type = piece;

    const float SCALE = 0.95; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
    const float SQ_SIZE = fminf(sec.width, sec.height) / 8.f;
    float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
    float pos[2] = {
        sec.x + col * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f),
        sec.y + row * SQ_SIZE + ((SQ_SIZE - texture_dim[1]) / 2.0f),
    };
    DrawTexturePro(*tex,
                   (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                   (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
}

void draw_promotion_options(Texture* tex, bool is_white, Rectangle sec)
{
    Vector2 rect_size = { sec.width * 0.6f, sec.height * 0.15f };
    Vector2 center = {
        (sec.x + sec.width / 2.f) - (rect_size.x / 2.f),
        (sec.y + sec.height / 2.f) - (rect_size.y / 2.f)
    };
    DrawRectangleV(
        center,
        rect_size,
        (Color) { 20, 20, 20, 150 }
    );

    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    for (int i = 0; i < 4; i++) {
        float frame_width = tex->width / 6.0f;   // Index from all piece types
        float frame_height = tex->height / 2.0f; // Index for white or black
        int color = is_white;
        int type = i + 1;

        const float SCALE = 0.85; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
        const float SQ_SIZE = rect_size.x * 0.25f;
        float texture_dim[2] = {SQ_SIZE * SCALE, SQ_SIZE * SCALE};
        float pos[2] = { center.x + i * SQ_SIZE + ((SQ_SIZE - texture_dim[0]) / 2.0f), center.y };
        DrawTexturePro(*tex,
                       (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                       (Rectangle){pos[0], pos[1], texture_dim[0], texture_dim[1]}, (Vector2){0, 0}, 0, WHITE);
    }
}

void choose_promotion_piece(GUI_Board* gb, Rectangle sec)
{
    Vector2 rect_size = { sec.width * 0.6f, sec.height * 0.15f };
    Vector2 center = {
        (sec.x + sec.width / 2.f) - (rect_size.x / 2.f),
        (sec.y + sec.height / 2.f) - (rect_size.y / 2.f)
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

void draw_eval_bar(Font font, float eval, Rectangle sec)
{
    // Function used:
    //   f(x) = (0.1443 * sqrt(|x|)) + 0.5
    //      where x = eval
    const float MAGIC_FACTOR = 0.1443f;
    float scale = sqrtf(fabsf(eval)) * MAGIC_FACTOR;
    scale *= eval < 0 ? 1 : -1;
    scale += 0.5;

    Color black = (Color) { 15, 15, 15, 255 };
    Color white = RAYWHITE;
    DrawRectangleV(
        (Vector2) { sec.x, sec.y },
        (Vector2) { sec.width, sec.height * scale },
        black
    );

    DrawRectangleV(
        (Vector2) { sec.x, sec.y + sec.height * scale },
        (Vector2) { sec.width, sec.height * (1 - scale) },
        white
    );

    const char* text = TextFormat("%.1f", fabsf(eval));
    float font_size = font.baseSize * 0.5f;
    Vector2 text_dim = MeasureTextEx(font, text, font_size, 0);
    Vector2 text_pos = { sec.x + sec.width / 2.f - text_dim.x / 2.f, 0 };
    Color text_color = { 0 };
    if (eval > 0) {
        // White is winning or has an advantage
        text_pos.y = sec.y + sec.height - (0.01 * sec.height + text_dim.y);
        text_color = black;
    } else {
        // Black is winning or has an advantage
        text_pos.y = sec.y + 0.01f * sec.height;
        text_color = white;
    }

    DrawTextEx(font, text, text_pos, font_size, 0, text_color);
}

void update_board_section(GUI* gui, GUI_Board* gb, Rectangle sec)
{
    float horz_padding = sec.width * 0.01f;
    float vert_padding = sec.height * 0.03f;

    // EVALUATION BAR
    const float MIN_SIDE_LEN = fminf(sec.width, sec.height);
    const float EVAL_BAR_HEIGHT = MIN_SIDE_LEN - (2 * vert_padding);
    gui->eval_boundary = (Rectangle) {
        .x = horz_padding + ((2 * horz_padding) + (sec.width * 0.05f)) / 2,
        .y = vert_padding + (((sec.height - vert_padding) / 2) - (EVAL_BAR_HEIGHT / 2)),
        .width = sec.width * 0.05f,
        .height = EVAL_BAR_HEIGHT,
    };

    // BOARD
    const float SQ_AREA = fminf(
        sec.width - (gui->eval_boundary.x + gui->eval_boundary.width) - (2 * horz_padding),
        gui->eval_boundary.height
    );

    float eval_and_pad = gui->eval_boundary.x + gui->eval_boundary.width + horz_padding;
    gui->board_boundary = (Rectangle) {
        .x = eval_and_pad + (((sec.width - eval_and_pad) / 2) - (SQ_AREA / 2)),
        .y = gui->eval_boundary.y,
        .width = SQ_AREA,
        .height = SQ_AREA,
    };

    if (gb->is_promotion)
        choose_promotion_piece(gb, gui->board_boundary);
    gui_board_update(gb, NULL, gui->board_boundary);
}


void draw_board_section(const GUI* const gui, const GUI_Board* const gb, Texture piece_tex, Font font)
{
    if (gui->eval_enabled) {
        draw_eval_bar(font, gui->eval, gui->eval_boundary);
        DrawRectangleLinesEx(gui->eval_boundary, 1.f, MAROON);
    }

    // BOARD
    draw_board(gb, gui->board_boundary, font);
    DrawRectangleLinesEx(gui->board_boundary, 1.f, DARKBLUE);

    for (int i = 0; i < 64; i++) {
        draw_piece(&piece_tex, pos_get_piece(gb->board.pos, i), ROW(i), COL(i), gui->board_boundary);
    }
    if (gb->is_promotion) {
        draw_promotion_options(&piece_tex, gb->board.state.side, gui->board_boundary);
    }
}

void draw_engine_section(Font font, Rectangle sec)
{
    (void) font;
    (void) sec;
}

int gui_main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetExitKey(KEY_Q);
    SetTargetFPS(30);

    GUI gui = { 0 };
    gui.eval_enabled = true;
    GUI_Board gb = { 0 };
    // STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    gui_board_init(&gb, "8/8/1k6/8/p3K3/8/8/8 w - - 0 1");

    Texture2D piece_tex = LoadTexture("assets/neo-pieces-spritesheet.png");

    Font primary_font = LoadFontEx("assets/font/Rubik-Regular.ttf", 30, NULL, 0);
    Font bold_font = LoadFontEx("assets/font/Rubik-Bold.ttf", 30, NULL, 0);

    Rectangle board_boundary = { 0 };
    Rectangle engine_boundary = { 0 };
    while (!WindowShouldClose()) {
        board_boundary = (Rectangle) {
            .x = 0,
            .y = 0,
            .width = GetRenderWidth() * 0.6f,
            .height = GetRenderHeight(),
        };

        engine_boundary = (Rectangle) {
            .x = board_boundary.x + board_boundary.width,
            .y = 0,
            .width = GetRenderWidth() * 0.4f,
            .height = GetRenderHeight(),
        };
        {
            update_board_section(&gui, &gb, board_boundary);
        }

        BeginDrawing();
        {
            ClearBackground(BACKGROUND);

            draw_board_section(&gui, &gb, piece_tex, bold_font);
            DrawRectangleLinesEx(board_boundary, 1.f, LIME);

            draw_engine_section(primary_font, engine_boundary);
            DrawRectangleLinesEx(engine_boundary, 1.f, SKYBLUE);
        }
        EndDrawing();
    }

    // De-initialization
    UnloadFont(primary_font);
    UnloadFont(bold_font);
    UnloadTexture(piece_tex);
    CloseWindow();

    return 0;
}

