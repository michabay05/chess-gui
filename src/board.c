#include "board.h"
#include <string.h>

void pos_add_piece(Position* pos, Piece piece, Sq sq) {
	set_bit(pos->piece[piece], sq);	
	pos_update_units(pos);
}

void pos_remove_piece(Position* pos, Piece piece, Sq sq) {
	pop_bit(pos->piece[piece], sq);	
	pos_update_units(pos);
}

Piece pos_get_piece(Position* pos, Sq sq) {
	for (int i = 0; i <= dK; i++) {
		if (get_bit(pos->piece[i], sq)) {
			return i;
		}
	}
	return E;
}

void pos_update_units(Position* pos) {
	// Reset all the units bitboard to 0 
	memset(pos->units, 0, sizeof(pos->units));
	for (int i = PAWN; i <= KING; i++) {
		pos->units[LIGHT] = pos->piece[i];
		pos->units[DARK] = pos->piece[6 + i];
	}
	pos->units[BOTH] = pos->units[LIGHT] | pos->units[DARK];
}

void change_side(State* state) {
	state->side = !state->side;
}

void board_set_from_fen(Board* board, FENInfo fen) {
	*board = (Board) {0};
	// Set pieces
	for (int i = 0; i < 64; i++) {
		if (fen.board[i] == E) continue;
		pos_add_piece(&board->pos, fen.board[i], i);
	}
	board->state.side = fen.side;
	board->state.castling = fen.castling;
	board->state.enpassant = fen.enpassant;
	board->state.half_moves = fen.half_moves;
	board->state.full_moves = fen.full_moves;
}
