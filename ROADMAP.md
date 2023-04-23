# ROADMAP
## GUI
- [x] Board
  - [x] Draw grid
  - [x] Color each grid by exchanging the light and dark colors
- [x] Piece
  - [x] Find pieces online
  - [x] Draw them centered in each square
- [ ] Controls
  - [x] Hightlight selected piece
	- [x] Hightlight preview possible moves
  - [ ] Drag piece to new square
  - [ ] Click and press piece to new square

## Rules
- [x] Board
  - [x] Piece bitboards
  - [x] Units bitboards
  - [x] Handle states
  - [x] Set commands from FEN strings
- [x] Precalculate attacks
  - [x] Leaper pieces
    - [x] Pawn
    - [x] Knight
    - [x] King
  - [x] Sliding pieces
		- [x] Magic bitboards
			- [x] Finding magic numbers
			- [x] Hard coding those magic constants
    - [x] Bishop
    - [x] Rook
    - [x] Queen(combination of rook and bishop)
- [x] Moves
	- [x] Encoding moves as a 32-bit integer
	- [x] Decoding move information
	- [x] Converting moves to strings
	- [x] Parsing moves from string

## Parsing
- [x] FEN
	- [x] Parse piece placement
	- [x] Parse side to move
	- [x] Parse castling rights
	- [x] Parse enpassant square
	- [x] Parse half moves
	- [x] Parse full moves
- [ ] PGN
- [ ] UCI
