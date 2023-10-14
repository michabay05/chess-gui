# ROADMAP

## GUI MODE

- [x] Board
  - [x] Draw grid
  - [x] Color each grid by exchanging the light and dark colors
- [x] Piece
  - [x] Find pieces online
  - [x] Draw them centered in each square
- [x] Controls
  - [x] Hightlight selected piece
  - [x] Hightlight preview possible moves
  - [x] Move piece
    - [x] Have a functioning method to make moves
    - [x] Highlight _legal_ target square for the piece
    - [x] When target square is clicked, move piece there
- [x] Show Evaluation bar
- [ ] Show list of moves played
- [ ] Export
  - [ ] FEN
  - [ ] PGN

## TERMINAL MODE

- Coming in the future . . .

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
    - [x] Bishop
    - [x] Rook
    - [x] Queen(combination of rook and bishop)
- [x] Moves
  - [x] Encoding moves as a 32-bit integer
  - [x] Decoding move information
  - [x] Converting moves to strings
  - [x] Parsing moves from string
  - [x] Make moves
    - [x] Move piece from source to target square
    - [x] Deal with captures, promotions, enpassant moves and castling
    - [x] Check if the currently made move results in a check - (If yes, unmake move)

## Parsing

- [x] FEN
  - [x] Parse piece placement
  - [x] Parse side to move
  - [x] Parse castling rights
  - [x] Parse enpassant square
  - [x] Parse half moves
  - [x] Parse full moves
- [ ] Move notations
  - [ ] SAN - Short Algebraic notation
  - [ ] Coordinate notation
- [ ] PGN
  - [x] Parse header section
  - [x] Parse move section (without comments)
  - [ ] Parse move section (with comments)
- [ ] UCI
  - [UCI commands notes](https://gist.github.com/aliostad/f4470274f39d29b788c1b09519e67372)
