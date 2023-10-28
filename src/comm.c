#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define INVALID_FD -1

typedef enum {
    IVT_DEPTH,
    IVT_SCORE,
    IVT_NODE_COUNT,
    IVT_TIME,
    IVT_PV_LINE,
} InfoValueType;

typedef struct {
    int depth;
    int score;
    int nodes;
    int nps;
    int time;
    char* pv;
} EngineOutput;

typedef struct {
    int in_fd;
    int out_fd;
    EngineOutput output;
} Engine;

// 1024 => a kilobyte
// n * 1024 => n kilobyte(s)
#define BUF_SIZE 1024 * 128


// Source of the 'bi_popen()' function:
//      https://unix.stackexchange.com/questions/606861/programming-communicating-with-chess-engine-stockfish-fifos-bash-redirecti
int bi_popen(const char* const command, int* const in, int* const out)
{

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

    const pid_t pid = fork();
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

void send_msg(Engine engine, char* msg)
{
    write(engine.out_fd, msg, strlen(msg));
}

size_t read_from_engine(Engine engine, char* buf, size_t buf_size)
{
    memset(buf, 0, buf_size);
    return read(engine.in_fd, buf, buf_size);
}

void close_engine(Engine engine)
{
    close(engine.in_fd);
    close(engine.out_fd);
}

#define peek(buf, size, ind) peek_ahead(buf, size, ind, 0)
#define peek_next(buf, size, ind) peek_ahead(buf, size, ind, 1)

static bool is_at_end(size_t size, size_t ind)
{
    return ind >= size;
}

static char peek_ahead(char* buf, size_t size, size_t ind, size_t ahead)
{
    if (is_at_end(size, ind)) return '\0';
    return buf[ind + ahead];
}

static char consume(char* buf, size_t size, size_t* ind)
{
    (*ind)++;
    if (is_at_end(size, *ind)) return '\0';
    char output = buf[(*ind) - 1];
    return output;
}


static bool str_n_append(char** dest, char* src, size_t size)
{
    if (src == NULL || dest == NULL) return false;

    size_t dest_size = *dest == NULL ? 0 : strlen(*dest);
    size_t src_size = strlen(src);

    if (src_size < size) return false;

    size_t new_size = dest_size + size + 1;
    char* temp = (char*) malloc(new_size * sizeof(char));
    if (temp == NULL) {
        return false;
    }

    memcpy(temp, *dest, dest_size);
    memcpy(temp + dest_size, src, size);
    temp[new_size - 1] = '\0';
    *dest = temp;
    return true;
}

static void consume_while(char** dest, char* buf, size_t size, size_t* i, bool (*filter_func)(char c)) {
    size_t prev_ind = *i;
    size_t word_len = 0;
    while (filter_func(peek(buf, size, *i))) {
        consume(buf, size, i);
        word_len++;
    }
    if (dest != NULL)
        str_n_append(dest, buf + prev_ind, word_len);
}


bool identify_info(char* info_key, InfoValueType* ivt)
{
    char* match[] = { "depth", "score", "nodes", "time", "pv" };

    for (size_t i = 0, n = (sizeof(match) / sizeof(match[0])); i < n; i++) {
        if (!strncmp(info_key, match[i], strlen(match[i]))) {
            *ivt = (InfoValueType) i;
            return true;
        }
    }

    return false;
}

void parse_engine_output(char* buf, size_t size)
{
    bool is_space(char c) { return isspace(c); }
    bool is_alnum(char c) { return isalnum(c); }

    printf("[INFO] Initial string: '%s'\n", buf);

    size_t i = 0;
    char* temp = NULL;
    char* key = NULL;
    char* value = NULL;
    int is_mate_score = 0;
    InfoValueType ivt;
    while (i < size) {
        temp = NULL;
        char c = peek(buf, size, i);
        if (isalnum(c)) {
            consume_while(&temp, buf, size, &i, &is_alnum);
            // printf("[INFO] Consumed: '%s'\n", temp);
            if (!strncmp(temp, "info", 4)) {
                printf("[INFO] info detected!\n");
                continue;
            }
            if (!strncmp(temp, "mate", 4) || !strncmp(temp, "cp", 2)) {
                is_mate_score = !strncmp(temp, "mate", 4) ? 1 : 0;
                printf("[INFO] is_mate_score is updated!\n");
                continue;
            }
            if (key == NULL) {
                key = temp;
                continue;
            } else {
                value = temp;
                printf("[INFO]\t\t\tkey = '%s', value = '%s'\n", key, value);
            }
            if (identify_info(key, &ivt)) {
                switch (ivt) {
                    case IVT_DEPTH:
                    case IVT_NODE_COUNT:
                    case IVT_TIME:
                        printf("[INFO] %s = %d\n", key, atoi(value));
                        break;
                    case IVT_PV_LINE:
                        break;
                    case IVT_SCORE:
                        if (is_mate_score == 0) printf("[INFO] Score is a 'cp' score.\n");
                        printf("[INFO] %s = %d\n", key, atoi(value));
                        break;
                }
                free(key);
                key = NULL;
                free(value);
            } else {
                // fprintf(stderr, "[ERROR] Ignored '%s'.\n", key);
                free(key);
                key = NULL;
            }
        } else if (isspace(c)) {
            if (c == '\n' || c == '\r') {
                printf("[ INFO] Encountered a newline, so I stopped.\n"); 
            }
            consume_while(&temp, buf, size, &i, &is_space);
        }
    }

    free(temp);
}

int main(void)
{
#if 1
#if 0
    FILE* fp = fopen("engine-output.txt", "r");
    char buf[BUF_SIZE] = { 0 };
    fread(buf, BUF_SIZE, sizeof(char), fp);
#endif


    char* buf = "info depth 18 seldepth 23 multipv 1 score cp 35 nodes 366649 nps 282472 hashfull 143 tbhits 0 time 1298 pv e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 g8f6 d4c6 b7c6 f1d3 d7d5 e4e5 f6d7 e1g1 f8e7 f2f4 d7c5 c1e3 e8g8";

    parse_engine_output(buf, strlen(buf));
#else
    Engine engine;
    char *line = (char*) malloc(BUF_SIZE * sizeof(char));

    const int pid = bi_popen("engines/stockfish", &engine.in_fd, &engine.out_fd);
    if (pid < 0) {
        perror("bi_popen");
        return 1;
    }

    if (engine.out_fd == INVALID_FD || engine.in_fd == INVALID_FD)
        return 1;

    read_from_engine(engine, line, BUF_SIZE);
    // printf("[ READ] %s", line);
    printf("====================================\n");
    printf("====================================\n");

    char* command = "uci\n";
    // printf("[WRITE] %s", command);
    send_msg(engine, command);
    read_from_engine(engine, line, BUF_SIZE);
    // printf("[ READ] %s", line);
    printf("====================================\n");
    printf("====================================\n");

    command = "position startpos\n";
    // printf("[WRITE] %s", command);
    send_msg(engine, command);
    // printf("[ INFO] No response.\n");
    printf("====================================\n");
    printf("====================================\n");

    command = "go movetime 1000\n";
    // printf("[WRITE] %s", command);
    send_msg(engine, command);

    // sleep for (1 second or 1000 ms) to wait for a response
    sleep(1);

    size_t bytes_read = read_from_engine(engine, line, BUF_SIZE);
    // printf("[ READ] %s", line);
    parse_engine_output(line, bytes_read);
    printf("====================================\n");
    printf("====================================\n");

    free(line);
    close_engine(engine);

#endif
    return 0;
}

