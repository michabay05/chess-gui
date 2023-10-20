#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 64 * 1024

typedef enum {
    PGN_TK_NONE,
    PGN_TK_TAG_KEY,
    PGN_TK_TAG_VALUE,
    PGN_TK_MOVE,
    PGN_TK_COMMENT,
    PGN_TK_GAME_OUTCOME,
} PGN_TokenKind;

typedef enum {
    PGN_GR_WHITE_WINS,
    PGN_GR_BLACK_WINS,
    PGN_GR_DRAW,
    PGN_GR_ONGOING,
} PGN_GameResult;

typedef struct {
    char* lexeme;
    PGN_TokenKind kind;
} PGN_Token;

typedef struct {
    PGN_Token* tokens;
    size_t count;
} PGN_TokenList;

typedef struct {
    char* event;
    char* site;
    char* date;
    char* round;
    char* white;
    char* black;
    PGN_GameResult result;
} PGN;

PGN_GameResult pgn_parse_result(char* lexeme);
void pgn_parse(PGN* pgn, const PGN_TokenList tl);
void pgn_print(const PGN* const pgn);
int pgn_main(void);
