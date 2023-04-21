#include "defs.h"
#include "gui_defs.h"

#include "raylib.h"

void draw_board() {
	float padding[2] = {
		(SCREEN_WIDTH - (8 * SQ_SIZE)) / 2.0f,	
		(SCREEN_HEIGHT - (8 * SQ_SIZE)) / 2.0f,	
	};
	for (int r = 0; r < 8; r++) {
		for (int f = 0; f < 8; f++) {
			int curr_sq = SQ(r, f);	
			Color sq_clr = SQCLR(r, f) ? LIGHT_CLR : DARK_CLR;
			DrawRectangleV(
					(Vector2) { f * SQ_SIZE + padding[0], r * SQ_SIZE + padding[1] },
					(Vector2) { SQ_SIZE, SQ_SIZE },
					sq_clr
				);
		}
	}	
}

int gui_main(void) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess GUI");

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(BACKGROUND);
			draw_board();
		EndDrawing();
	}
	CloseWindow();

	return 0;
}

