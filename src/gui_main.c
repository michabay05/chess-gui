#include "defs.h"
#include "gui_defs.h"

#include "raylib.h"

const float padding[2] = {
	(SCREEN_WIDTH - (8 * SQ_SIZE)) / 2.0f,
	(SCREEN_HEIGHT - (8 * SQ_SIZE)) / 2.0f,
};

void draw_board() {
	for (int r = 0; r < 8; r++) {
		for (int f = 0; f < 8; f++) {
			Color sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
			DrawRectangleV(
					(Vector2) { f * SQ_SIZE + padding[0], r * SQ_SIZE + padding[1] },
					(Vector2) { SQ_SIZE, SQ_SIZE },
					sq_clr
				);
		}
	}	
}

static int get_correct_piece(Piece piece) {
	int type = COLORLESS(piece);
	switch (type) {
		case PAWN:
			type = 5;
			break;
		case KNIGHT:
			type = 3;
			break;
		case BISHOP:
			type = 6;
			break;
		case ROOK:
			type = 4;
			break;
		case QUEEN:
			type = 2;
			break;
		case KING:
			type = 1;
			break;
	}
	return type;
}

void draw_pieces(Texture2D* tex, int row, int col) {
	SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
	int piece = lQ;
	int frame_width = tex->width / 6; // INDEX FROM ALL PIECE TYPES
	int frame_height = tex->height / 2; // INDEX FOR WHITE OR BLACK
	int color = piece >= 6; 
	int type = get_correct_piece(piece);

	DrawTexturePro(
			*tex,
			(Rectangle) {type * frame_width, color * frame_height, frame_width, frame_height},
			(Rectangle) {col * SQ_SIZE + padding[0], row * SQ_SIZE + padding[1], SQ_SIZE, SQ_SIZE},
			(Vector2) {(float)SQ_SIZE / 2, (float)SQ_SIZE / 2},
			0,
			WHITE	
		);
}

int gui_main(void) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");

	Texture2D tex = LoadTexture("assets/pieces.png");

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(BACKGROUND);

			draw_board();
			draw_pieces(&tex, 4, 4);

		EndDrawing();
	}
	UnloadTexture(tex);
	CloseWindow();

	return 0;
}

