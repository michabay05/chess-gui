#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 64 * 1024

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
    while (!isspace((c = peek(buf, size, i)))) {
        if (is_valid_move_letter(c)) return true;
        else consume(buf, size, &i);
    }
    return false;
}

void parse_lines(char* buf)
{
    const size_t size = strlen(buf);
    if (buf == NULL || size == 0) return;
    
    // @TODO: something is wrong with consume()
    // It can't appropriately consume spaces and newlines; therefore,
    // on the first PNG metadata -- EVENT -- is consumed. Then, it's stuck
    // in an infinite loop

    size_t i = 0;
    while (!is_at_end(size, i)) {
        char c = peek(buf, size, i);
        if (c == '[') {
            // [KEY "VALUE"
            // PGN meta data
            consume(buf, size, &i); // Consume [
            // Consumes the entire word of the KEY
            while (peek(buf, size, i) != ' ') {
                printf("%c", consume(buf, size, &i));
            }
            consume(buf, size, &i); // Consume ' '

            printf("\n");

            consume(buf, size, &i); // Consume '"'
            while (peek(buf, size, i) != '"') {
                printf("%c", consume(buf, size, &i));
            }
            consume(buf, size, &i); // Consume '"'

            consume(buf, size, &i); // Consume the closing square bracket mark
            printf("\n");
        } else if (is_move_number(buf, size, i)) {
            printf("\n");
            while (isdigit(peek(buf, size, i))) { printf("%c", consume(buf, size, &i)); }
            printf(".  ");
            consume(buf, size, &i); // Consume the period after the move number
            consume(buf, size, &i); // Consume the space after the move number's period
        } else if (is_move_text(buf, size, i)) {
            while (!isspace(peek(buf, size, i))) {
                printf("%c", consume(buf, size, &i));
            }
            printf("\t");
        } else if (isspace(c)) {
            consume(buf, size, &i);
        }
    }
    // printf("[INFO] Done!\n");
}

int main(void)
{
    const char* file_path = "examples/pgn_without_comments.txt";

    FILE* fptr = fopen(file_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "[ERROR] Failed to open '%s'.\n", file_path);
        return 1;
    }

    char buf[BUF_SIZE] = { 0 };
    while (fgets(buf, BUF_SIZE, fptr)) {
        if (strncmp(buf, "quit", 4) == 0) break;
        parse_lines(buf);
    }

    printf("[INFO] Done!\n");
    return 0;
}
