#pragma once

typedef enum {
    EST_CP = 1,
    EST_MATE,
} EngineScoreType;

typedef struct {
    int depth;
    EngineScoreType score_type;
    int score;
    int nodes;
    int time;
    char* pv;
} EngineOutput;

typedef struct {
    int read_fd;
    int write_fd;
    EngineOutput* output;
    size_t output_size;
    Move best_move;
} Engine;

bool open_engine(char* filepath, Engine* engine);
void close_engine(Engine engine);
void send_to_engine(Engine engine, char* command);
void read_from_engine(Engine engine, char* buf, size_t size);
void parse_engine_output(char* buf, size_t size, Engine* engine);
