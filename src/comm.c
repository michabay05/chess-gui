#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "move.h"
#include "utils.h"
#include "comm.h"

#define INVALID_FD -1

typedef enum {
    IVT_DEPTH =  1,
    IVT_SCORE,
    IVT_NODES,
    IVT_TIME,
    IVT_PV_LINE,
} InfoValueType;

static void engine_add_output(Engine* engine, EngineOutput eo)
{
    engine->output_size++;
    engine->output = (EngineOutput*) realloc(engine->output, engine->output_size * sizeof(EngineOutput));

    memcpy(&engine->output[engine->output_size - 1], &eo, sizeof(EngineOutput));
}

/* ================== UTIL FUNCTIONS ================ */
bool is_space(char c) { return isspace(c); }
// is alphabetic or numeric
bool is_alnum(char c) { return isalnum(c); }
// is not end-of-line
bool is_not_eol(char c) { return c != '\r' && c != '\n'; }
/* ================================================== */

static void parse_config(Engine* engine, char* config, size_t size)
{

    size_t i = 0;
    char c;
    char* temp = NULL;
    char* key = NULL;
    while (i < size) {
        c = peek(config, size, i);
        if (isalnum(c)) {
            // id name Stockfish
            // id
            consume_while(&temp, config, size, &i, &is_alnum);
            if (!strncmp(temp, "uciok", 5)) {
                engine->uci_ok = true;
            } else if (!strncmp(temp, "id", 2)) {
                consume_while(NULL, config, size, &i, &is_space);
                // name
                consume_while(&temp, config, size, &i, &is_alnum);
                if (!strncmp(temp, "name", 4)) {
                    consume_while(NULL, config, size, &i, &is_space);
                    // <ENGINE_NAME>
                    consume_while(&engine->name, config, size, &i, &is_not_eol);
                } else {
                    consume_while(NULL, config, size, &i, &is_space);
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    consume_while(NULL, config, size, &i, &is_space);
                    printf("[INFO] '%s'\n", config + i);
                    // exit(1234);
                }
            } else if (!strncmp(temp, "option", 6)) {
                // 'option name Threads type spin default 1 min 1 max 1024'
                // name
                consume_while(NULL, config, size, &i, &is_space);
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "name", 4)) {
                    // if the word after `option` isn't `name`, then skip the entire line and move on
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }
                consume_while(NULL, config, size, &i, &is_space);
                // Threads
                consume_while(&key, config, size, &i, &is_alnum);

                consume_while(NULL, config, size, &i, &is_space);
                // type
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "type", 4)) {
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }

                consume_while(NULL, config, size, &i, &is_space);
                // spin
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "spin", 4)) {
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }
                
                consume_while(NULL, config, size, &i, &is_space);
                // default
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "default", 7)) {
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }
                consume_while(NULL, config, size, &i, &is_space);
                // 1
                consume_while(&temp, config, size, &i, &is_alnum);
                int default_size = atoi(temp);

                consume_while(NULL, config, size, &i, &is_space);
                // min
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "min", 3)) {
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }
                consume_while(NULL, config, size, &i, &is_space);
                // 1
                consume_while(&temp, config, size, &i, &is_alnum);
                int min_size = atoi(temp);

                consume_while(NULL, config, size, &i, &is_space);
                // max
                consume_while(&temp, config, size, &i, &is_alnum);
                if (strncmp(temp, "max", 3)) {
                    consume_while(NULL, config, size, &i, &is_not_eol);
                    continue;
                }
                consume_while(NULL, config, size, &i, &is_space);
                // 1024
                consume_while(&temp, config, size, &i, &is_alnum);
                int max_size = atoi(temp);

                if (!strncmp(key, "Threads", 7)) {
                    engine->threads = default_size;
                    engine->min_threads = min_size;
                    engine->max_threads = max_size;
                } else if (!strncmp(key, "Hash", 4)) {
                    engine->hash_size = default_size;
                    engine->min_hash_size = min_size;
                    engine->max_hash_size = max_size;
                }
            }
        } else if (isspace(c)) {
            consume_while(NULL, config, size, &i, &is_space);
        }
    }

    printf("[INFO]          name = '%s'\n", engine->name);
    printf("[INFO]       threads = '%d'\n", engine->threads);
    printf("[INFO]   min_threads = '%d'\n", engine->min_threads);
    printf("[INFO]   max_threads = '%d'\n", engine->max_threads);
    printf("[INFO]     hash_size = '%d'\n", engine->hash_size);
    printf("[INFO] min_hash_size = '%d'\n", engine->min_hash_size);
    printf("[INFO] max_hash_size = '%d'\n", engine->max_hash_size);

    free(temp);
}

void read_from_engine(Engine engine, char* buf, size_t size)
{
    read(engine.read_fd, buf, size);
}

void send_to_engine(Engine engine, char* command)
{
    write(engine.write_fd, command, strlen(command));
}

// This function is defined at the end of this file
// Its prototype is here because it's used in the function below
static int bi_popen(const char* const command, int* const in, int* const out);

bool load_engine(Engine* engine, const char* filepath)
{
    *engine = (Engine) { 0 };
    const int pid = bi_popen(filepath, &engine->read_fd, &engine->write_fd);
    if (pid < 0) {
        perror("bi_popen");
        return false;
    }

    if (engine->read_fd == INVALID_FD || engine->write_fd == INVALID_FD)
        return false;

    const int BUF_SIZE = 8 * 1024;
    char buf[8 * 1024] = { 0 };

    // Read whatever the engine prints out at the beginning
    // Don't need it
    read_from_engine(*engine, buf, BUF_SIZE);

    send_to_engine(*engine, "uci\n");

    memset(buf, 0, BUF_SIZE);
    read_from_engine(*engine, buf, BUF_SIZE);
    parse_config(engine, buf, strlen(buf));

    send_to_engine(*engine, "isready\n");

    memset(buf, 0, BUF_SIZE);
    read_from_engine(*engine, buf, BUF_SIZE);

    // If engine doesn't respond with 'readyok', then
    // the engine is pressumed to be unprepared
    return engine->uci_ok && (!strncmp(buf, "readyok", 7));
}

void unload_engine(Engine engine)
{
    close(engine.read_fd);
    close(engine.write_fd);
    free(engine.output);
    free(engine.name);
}

static bool identify_info(char* info_key, InfoValueType* ivt)
{
    char* match[] = { "depth", "score", "nodes", "time", "pv", "bestmove" };
    InfoValueType ivt_match[] = {
        IVT_DEPTH, IVT_SCORE, IVT_NODES, IVT_TIME, IVT_PV_LINE, IVT_BEST_MOVE
    };

    for (size_t i = 0, n = (sizeof(match) / sizeof(match[0])); i < n; i++) {
        if (!strncmp(info_key, match[i], strlen(match[i]))) {
            *ivt = ivt_match[i];
            return true;
        }
    }

    return false;
}

void parse_engine_output(char* buf, size_t size, Engine* engine)
{
    printf("[INFO] Initial string: '%s'\n", buf);

    size_t i = 0;
    char* temp = NULL;
    char* key = NULL;
    char* value = NULL;
    int is_mate_score = 0;
    InfoValueType ivt;
    EngineOutput output = { 0 };
    while (i < size) {
        free(temp);
        temp = NULL;

        char c = peek(buf, size, i);
        if (isalnum(c)) {
            if (key != NULL && !strncmp(key, "pv", 2)) {
                consume_while(&temp, buf, size, &i, &is_not_eol);
            } else {
                consume_while(&temp, buf, size, &i, &is_alnum);
                if (!strncmp(temp, "info", 4)) {
                    continue;
                }
                if (!strncmp(temp, "mate", 4) || !strncmp(temp, "cp", 2)) {
                    is_mate_score = !strncmp(temp, "mate", 4) ? 1 : 0;
                    continue;
                }
            }
            if (key == NULL) {
                key = temp;
                continue;
            } else {
                value = temp;
            }
            if (identify_info(key, &ivt)) {
                switch (ivt) {
                    case IVT_DEPTH:
                        output.depth = atoi(value);
                        break;
                    case IVT_NODES:
                        output.nodes = atoi(value);
                        break;
                    case IVT_SCORE:
                        output.score_type = is_mate_score ? EST_MATE : EST_CP;
                        output.score = atoi(value);
                        break;
                    case IVT_TIME:
                        output.time = atoi(value);
                        break;
                    case IVT_PV_LINE:
                        output.pv = value;
                        // Append parsed output to save in a list
                        engine_add_output(engine, output);
                        // Reset output for next line
                        output = (EngineOutput) { 0 };
                        break;
                }
                free(key);
                key = NULL;
                value = NULL;
            } else {
                // Unknown key; ignored
                free(key);
                key = NULL;
            }
        } else if (isspace(c)) {
            consume_while(&temp, buf, size, &i, &is_space);
        }
    }

    free(temp);
    temp = NULL;
}


// ========================================================================================================================
// Source of the 'bi_popen()' function:
//      https://unix.stackexchange.com/questions/606861/programming-communicating-with-chess-engine-stockfish-fifos-bash-redirecti
static int bi_popen(const char* const command, int* const in, int* const out)
{
    const int READ_END = 0;
    const int WRITE_END = 1;


    int to_child[2] = { INVALID_FD, INVALID_FD };
    int to_parent[2] = { INVALID_FD, INVALID_FD };

    *in = INVALID_FD;
    *out = INVALID_FD;

    if (command == NULL || in == NULL || out == NULL) {
        errno = EINVAL;
        goto bail;
    }

    if (pipe(to_child) < 0) {
        goto bail;
    }

    if (pipe(to_parent) < 0) {
        goto bail;
    }

    const int pid = fork();
    if (pid < 0) {
        goto bail;
    }

    if (pid == 0) { // Child
        if (dup2(to_child[READ_END], STDIN_FILENO) < 0) {
            perror("dup2");
            exit(1);
        }
        close(to_child[READ_END]);
        close(to_child[WRITE_END]);

        if (dup2(to_parent[WRITE_END], STDOUT_FILENO) < 0) {
            perror("dup2");
            exit(1);
        }
        close(to_parent[READ_END]);
        close(to_parent[WRITE_END]);

        execlp(command, command, NULL);
        perror("execlp");
        exit(1);
    }

    // Parent
    close(to_child[READ_END]);
    to_child[READ_END] = INVALID_FD;

    close(to_parent[WRITE_END]);
    to_parent[WRITE_END] = INVALID_FD;

    *in  = to_parent[READ_END];
    *out = to_child[WRITE_END];

    return pid;

bail:
    ; // Goto label must be a statement, this is an empty statement
    const int old_errno = errno;

    for (int i = 0; i < 2; ++i) {
        if (to_child[i] != INVALID_FD) {
            close(to_child[i]);
        }
        if (to_parent[i] != INVALID_FD) {
            close(to_parent[i]);
        }
    }

    errno = old_errno;
    return -1;
}

