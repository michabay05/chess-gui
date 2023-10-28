#pragma once

typedef enum {
    PGN_GR_WHITE_WINS,
    PGN_GR_BLACK_WINS,
    PGN_GR_DRAW,
    PGN_GR_ONGOING,
} PGN_GameResult;

typedef struct {
    char* event;
    char* site;
    char* date;
    char* round;
    char* white;
    char* black;
    PGN_GameResult result;
} PGN;

void pgn_print(const PGN* const pgn);
bool pgn_read(char* filepath, PGN* pgn);
