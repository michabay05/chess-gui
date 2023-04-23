#pragma once

#include "defs.h"

typedef int32_t Move;

int move_encode(Sq source, Sq target, Piece piece, Piece promoted,
                bool isCapture, bool isTwoSquarePush, bool isEnpassant,
                bool isCastling);
int move_get_source(const int move);
int move_get_target(const int move);
int move_get_piece(const int move);
int move_get_promoted(const int move);
bool move_is_capture(const int move);
bool move_is_two_square_push(const int move);
bool move_is_enpassant(const int move);
bool move_is_castling(const int move);

void sq_to_str(Sq sq, char *str);
void move_to_str(int move, char* move_str);
int move_parse(char *move_str, Piece piece, bool isCapture,
               bool isTwoSquarePush, bool isEnpassant, bool isCastling);
