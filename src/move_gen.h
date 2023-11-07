#pragma once

#include "board.h"
#include "defs.h"
#include "move.h"

typedef struct {
    int list[256];
    int count;  
} MoveList;

void movelist_add(MoveList* ml, Move move);
Move movelist_search(const MoveList ml, Sq source, Sq target, Piece promoted);
void movelist_print_list(const MoveList ml);

void movelist_generate_all(MoveList* ml, const Board* const b);
void movelist_generate(MoveList* ml, const Board* const b, Piece p);
void movelist_legal(MoveList* ml, Board* b);
