#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 64 * 1024

#define peek(buf, size, curr_ind) peek_ahead(buf, size, curr_ind, 0)
#define peek_next(buf, size, curr_ind) peek_ahead(buf, size, curr_ind, 1)

char peek_ahead(char* buf, size_t buf_size, size_t curr_ind, size_t ahead)
{
    if (curr_ind >= buf_size) return '\0';
    return buf[curr_ind + ahead];
}

char consume(char* buf, size_t buf_size, size_t* curr_ind)
{
    (*curr_ind)++;
    if (*curr_ind >= buf_size) return '\0';
    char output = buf[(*curr_ind) - 1];
    return output;
}

void parse_lines(char* buf, size_t size)
{
    if (buf == NULL || size == 0) return;
    
    // @TODO: can't figure out why `consume()` isn't working properly in the move parsing section
    // It gets stuck on black's move after the white's move - 'Bd3'
    // It's probably because of end of buffer index control; The move - 'Bd3' - is the last move before the
    // newline. So the problem lies in the index control of `consume()` and its placement and order in the 
    // move parsing section

    size_t i = 0;
    char accepted_chars[] = {'-', '#', '+'};
    while (i < size) {
        char c = buf[i];
        if (c == '[') {
            // PGN meta data
            consume(buf, size, &i);
            while (i < size && peek(buf, size, i) != ']') {
                printf("%c", consume(buf, size, &i));
                if (c == '"') {
                    // Value of PGN meta data
                    printf("\t");
                    consume(buf, size, &i); // opening quotation mark
                    while (i < size && (isgraph(peek(buf, size, i)) && peek(buf, size, i) != '"')) {
                        printf("%c", consume(buf, size, &i));
                    }
                    consume(buf, size, &i); // closing quotation mark
                    printf("\n");
                }
            }
            printf("\n");
        } else if (isdigit(peek(buf, size, i)) && peek_next(buf, size, i) == '.') {
            while (peek(buf, size, i) != ' ') { consume(buf, size, &i); }
            consume(buf, size, &i); // Consume the space after the move number
            while (isalnum(peek(buf, size, i)) || peek(buf, size, i) == '-') {
                printf("%c", consume(buf, size, &i));
            }
            consume(buf, size, &i); // Consume the space between white's and black's move
            printf("\t\t");
            while (isalnum(peek(buf, size, i)) || strchr(accepted_chars, peek(buf, size, i)) != NULL) {
                printf("%c", consume(buf, size, &i));
            }
            printf("\n");
        } else if (isspace(c)) {
            break;
        }
        i++;
    }
}

int main(void)
{
    const char* file_path = "tests/test_pgn_without_comments.txt";

    FILE* fptr = fopen(file_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "[ERROR] Failed to open '%s'.\n", file_path);
        return 1;
    }

    char buf[BUF_SIZE] = { 0 };
    while (fgets(buf, BUF_SIZE, fptr)) {
        parse_lines(buf, BUF_SIZE);
    }

    return 0;
}
