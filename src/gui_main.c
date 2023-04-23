#include "defs.h"
#include "fen.h"
#include "gui_defs.h"

#include "raylib.h"

const float padding[2] = {
	(SCREEN_WIDTH - (8 * SQ_SIZE)) / 2.0f,
	(SCREEN_HEIGHT - (8 * SQ_SIZE)) / 2.0f,
};

void draw_board() {
	for (int r = 0; r < 8; r++) {
		for (int f = 0; f < 8; f++) {
			Color sq_clr = !SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
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
			type = 2;
			break;
		case ROOK:
			type = 4;
			break;
		case QUEEN:
			type = 1;
			break;
		case KING:
			type = 0;
			break;
	}
	return type;
}

void draw_pieces(Texture2D* tex, Piece piece ,int row, int col) {
	if (piece == E) return;
	SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
	float frame_width = tex->width / 6.0f; // INDEX FROM ALL PIECE TYPES
	float frame_height = tex->height / 2.0f; // INDEX FOR WHITE OR BLACK
	int color = piece >= 6;
	int type = get_correct_piece(piece);

	// float scale = SQ_SIZE / frame_width;
	float scale = 0.4875;
	float dim[2] = { frame_width * scale, frame_height * scale };
	float pos[2] = {
		col * SQ_SIZE + padding[0] + ((SQ_SIZE - dim[0]) / 2.0f),
		row * SQ_SIZE + padding[1] + ((SQ_SIZE - dim[1]) / 2.0f),
	};
	DrawTexturePro(
			*tex,
			(Rectangle) {type * frame_width, color * frame_height, frame_width, frame_height},
			(Rectangle) {pos[0], pos[1], dim[0], dim[1]},
      (Vector2) {0, 0},
			0,
			WHITE	
		);
}

int gui_main(void) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");

	Texture2D tex = LoadTexture("assets/piece_sprites.png");
	FENInfo fen = parse_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(BACKGROUND);

			draw_board();
			for (int i = 0; i < 64; i++) {
				draw_pieces(&tex, fen.board[i], ROW(i), COL(i));
			}

		EndDrawing();
	}
	UnloadTexture(tex);
	CloseWindow();

	return 0;
}

