#include "board.h"
#include "fen.h"
#include "gui_board.h"
#include "gui_defs.h"

const float padding[2] = {
    (SCREEN_WIDTH - (8 * SQ_SIZE)) / 2.0f,
    (SCREEN_HEIGHT - (8 * SQ_SIZE)) / 2.0f,
};

void draw_board(const GUI_Board gb)
{
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            Color sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
            if ((int)gb.selected == SQ(r, f) && pos_get_piece(gb.board.pos, gb.selected) != E)
                sq_clr = ColorAlphaBlend(sq_clr, SELECTED_COLOR, WHITE);
            if (get_bit(gb.preview, SQ(r, f)))
                sq_clr = ColorAlphaBlend(sq_clr, PREVIEW_COLOR, WHITE);
            DrawRectangleV((Vector2){f * SQ_SIZE + padding[0], r * SQ_SIZE + padding[1]},
                           (Vector2){SQ_SIZE, SQ_SIZE}, sq_clr);
        }
    }
}

void draw_pieces(Texture2D *tex, Piece piece, int row, int col)
{
    if (piece == E)
        return;
    SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
    float frame_width = tex->width / 6.0f;   // INDEX FROM ALL PIECE TYPES
    float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
    int color = piece >= 6;
    int type = piece;

    float scale = 0.4875; // MAGIC NUMBER I CHOSE BECAUSE IT LOOKS GOOD
    float dim[2] = {frame_width * scale, frame_height * scale};
    float pos[2] = {
        col * SQ_SIZE + padding[0] + ((SQ_SIZE - dim[0]) / 2.0f),
        row * SQ_SIZE + padding[1] + ((SQ_SIZE - dim[1]) / 2.0f),
    };
    DrawTexturePro(*tex,
                   (Rectangle){type * frame_width, color * frame_height, frame_width, frame_height},
                   (Rectangle){pos[0], pos[1], dim[0], dim[1]}, (Vector2){0, 0}, 0, WHITE);
}

int gui_main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");
    SetTargetFPS(30);

    Texture2D tex = LoadTexture("assets/neo-pieces-spritesheet.png");
    FENInfo fen = parse_fen("6R1/P2k4/r7/5N1P/r7/p7/7K/8 w - - 0 1");
    GUI_Board gb;
    gui_board_init(&gb);
    board_set_from_fen(&gb.board, fen);

    while (!WindowShouldClose()) {
        gui_board_update(&gb);
        BeginDrawing();
        {
            ClearBackground(BACKGROUND);

            draw_board(gb);
            for (int i = 0; i < 64; i++) {
                draw_pieces(&tex, pos_get_piece(gb.board.pos, i), ROW(i), COL(i));
            }
        }
        EndDrawing();
    }
    UnloadTexture(tex);
    CloseWindow();

    return 0;
}
