#pragma once

#include "gui_defs.h"
#include "defs.h"
#include "board.h"

#define SELECTED_COLOR (Color){ 130, 151, 105, 227 }
#define PREVIEW_COLOR  (Color) { 119, 139, 189, 200 }

typedef struct {
  Rectangle boundary;
  Board board;
  Sq selected;
  uint64_t preview;
} GUI_Board;

void gui_board_init(GUI_Board* gb);
void gui_board_update(GUI_Board* gb);
