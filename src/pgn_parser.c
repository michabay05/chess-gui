#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 64 * 1024

typedef enum {
    PGN_NONE,
    PGN_METADATA_KEY,
    PGN_METADATA_VALUE,
    PGN_MOVE,
    PGN_COMMENT,
} PGN_TokenKind;

typedef struct {
    char* buf;
    PGN_TokenKind kind;
} PGN_Token;

typedef struct {
    PGN_Token* tokens;
    size_t count;
} PGN_TokenList;

#define peek(buf, size, ind) peek_ahead(buf, size, ind, 0)
#define peek_next(buf, size, ind) peek_ahead(buf, size, ind, 1)

bool is_at_end(size_t size, size_t ind)
{
    return ind >= size;
}

char peek_ahead(char* buf, size_t size, size_t ind, size_t ahead)
{
    if (is_at_end(size, ind)) return '\0';
    return buf[ind + ahead];
}

char consume(char* buf, size_t size, size_t* ind)
{
    (*ind)++;
    if (is_at_end(size, *ind)) return '\0';
    char output = buf[(*ind) - 1];
    return output;
}

bool is_move_number(char* buf, size_t size, size_t ind)
{
    size_t i = ind;
    char c;
    while ((c = peek(buf, size, i)) != '.') {
        if (!isdigit(c)) return false;
        consume(buf, size, &i);
    }
    return true;
}

bool is_valid_move_letter(char c)
{
    if (c == '\0') return false;

    char* accepted_chars[5] = {
        "NBRQK",      // Piece types
        "abcdefgh",   // file letters
        "12345678",   // rank numbers
        "O0",         // Castling letter
        "-+#",        // Other symbols
    };
    for (int i = 0; i < 5; i++) {
        if (strchr(accepted_chars[i], c) != NULL) return true;
    }

    return false;
}

bool is_move_text(char* buf, size_t size, size_t ind)
{
    size_t i = ind;
    char c;
    while (c != '\0' && !isspace(c)) {
        if (is_valid_move_letter(c)) return true;
        else consume(buf, size, &i);

        c = peek(buf, size, i);
    }
    return false;
}

bool str_append_char(char** dest, char c)
{
    size_t size = *dest == NULL ? 0 : strlen(*dest);
    size_t new_size = size + 2; // + 2 for c and NUL 
    char* temp = (char*) malloc(new_size * sizeof(char));
    if (dest == NULL) {
        return false;
    }

    memcpy(temp, *dest, size);
    temp[new_size - 2] = c;
    temp[new_size - 1] = '\0';
    *dest = temp;
    return true;
}

void token_append(PGN_TokenList* tl, PGN_Token t)
{
    tl->count++;
    tl->tokens = (PGN_Token*) realloc(tl->tokens, tl->count * sizeof(PGN_Token));

    tl->tokens[tl->count - 1].buf = (char*) malloc(strlen(t.buf) * sizeof(char) + 1);
    memcpy(tl->tokens[tl->count - 1].buf, t.buf, strlen(t.buf));
    tl->tokens[tl->count - 1].buf[strlen(t.buf)] = '\0';
    tl->tokens[tl->count - 1].kind = t.kind;
}

void token_print(PGN_Token t)
{
    const char* kind_str;
    switch (t.kind) {
        case PGN_NONE:
            return;
        case PGN_METADATA_KEY:
            kind_str = "key";
            break;
        case PGN_METADATA_VALUE:
            kind_str = "value";
            break;
        case PGN_MOVE:
            kind_str = "move";
            break;
        case PGN_COMMENT:
            kind_str = "comment";
            break;
    }
    printf("[TOKEN: %7s] %s", kind_str, t.buf);
}

void token_reset(PGN_Token* t)
{
    t->buf = NULL;
    t->kind = 0;
}

void parse_lines(char* buf, PGN_TokenList* tl)
{
    const size_t size = strlen(buf);
    if (buf == NULL || size == 0) return;
    
    size_t i = 0;
    PGN_Token t = { 0 };
    while (!is_at_end(size, i)) {
        char c = peek(buf, size, i);
        if (c == '[') {
            // [KEY "VALUE"]
            // PGN meta data
            consume(buf, size, &i); // Consume [
            // Consumes the entire word of the KEY
            while (peek(buf, size, i) != ' ') {
                str_append_char(&t.buf, consume(buf, size, &i));
            }
            t.kind = PGN_METADATA_KEY;
            token_append(tl, t);
            token_reset(&t);

            consume(buf, size, &i); // Consume ' '
            consume(buf, size, &i); // Consume '"'

            while (peek(buf, size, i) != '"') {
                str_append_char(&t.buf, consume(buf, size, &i));
            }
            t.kind = PGN_METADATA_VALUE;
            token_append(tl, t);
            token_reset(&t);

            consume(buf, size, &i); // Consume '"'
            consume(buf, size, &i); // Consume the closing square bracket mark
        } else if (c == '{') {
            consume(buf, size, &i); // Consume '{'
            while (peek(buf, size, i) != '}') {
                str_append_char(&t.buf, consume(buf, size, &i));
            }
            t.kind = PGN_COMMENT;
            token_append(tl, t);
            token_reset(&t);
            consume(buf, size, &i); // Consume '}'
        } else if (is_move_number(buf, size, i)) {
            while (peek(buf, size, i) != '.') { consume(buf, size, &i); }
            while (peek(buf, size, i) == '.') { consume(buf, size, &i); }
        } else if (is_move_text(buf, size, i)) {
            while (!isspace(peek(buf, size, i))) {
                str_append_char(&t.buf, consume(buf, size, &i));
            }
            t.kind = PGN_MOVE;
            token_append(tl, t);
            token_reset(&t);
        } else if (isspace(c)) {
            consume(buf, size, &i);
        }
    }

}

int main(void)
{
    //const char* file_path = "examples/fischer_v_spassky_modified.pgn";
    const char* file_path = "examples/fischer_v_spassky.pgn";
    // const char* file_path = "examples/without_comments.pgn";

    FILE* fptr = fopen(file_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "[ERROR] Failed to open '%s'.\n", file_path);
        return 1;
    }

    char buf[BUF_SIZE] = { 0 };
    PGN_TokenList tl = { 0 };
    while (fgets(buf, BUF_SIZE, fptr)) {
        if (strncmp(buf, "quit", 4) == 0) break;
        parse_lines(buf, &tl);
    }

    for (size_t i = 0; i < tl.count; i++) {
        printf("%3ld. ", i + 1);
        token_print(tl.tokens[i]);
        printf("\n");
    }

    printf("[INFO] Done!\n");
    return 0;
}
