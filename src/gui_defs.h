#pragma once

#include "board.h"
#include "defs.h"
#include "move_gen.h"
#include "raylib.h"

#include <stdlib.h>  // For realloc(), file writing

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600

// Colors
#define BACKGROUND (Color) { 28, 28, 28, 255 }
#define LIGHT_CLR (Color) { 240, 217, 181, 255 }
#define DARK_CLR (Color) { 181, 136,  99, 255 }

// clang-format off
#define SELECTED_COLOR (Color) { 130, 151, 105, 227 }
#define PREVIEW_COLOR (Color) { 119, 139, 189, 200 }
// clang-format on

typedef struct
{
    Board board;
    bool is_promotion;
    Piece promoted_choice;
    MoveList ml;
    Sq selected;
    Sq target;
    uint64_t preview;
} GUI_Board;

typedef enum
{
    SECTION_EVAL_BAR,
    SECTION_BOARD,
    SECTION_MOVES_LIST,
    SECTION_INFO
} SectionType;

typedef struct
{
    Vector2 size;
    Vector2 padding;
} Section;

typedef enum
{
    GS_NORMAL,
    GS_CHECKMATE,
    GS_STALEMATE,
} GameState;

typedef struct
{
    // @REFACTOR: redefine sections to rectangles
    Section sections[4];
    Font font[3];
    GameState state;
    GUI_Board gb;

    FILE* fptr;
} Game;

// gui_main.c
int gui_main(void);

// gui_board.c
void gui_board_init(GUI_Board *gb);
void gui_board_update(GUI_Board *gb, FILE* record_fptr, Section sec);
