#include "pgn.h"

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
    while (c != '\0' && !isspace(c)) {
        if (is_valid_move_letter(c)) return true;
        else consume(buf, size, &i);

        c = peek(buf, size, i);
    }
    return false;
}

bool str_n_append(char** dest, char* src, size_t size)
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

bool str_append(char** dest, char* src)
{
    return str_n_append(dest, src, strlen(src));
}

void token_append(PGN_TokenList* tl, PGN_Token t)
{
    tl->count++;
    tl->tokens = (PGN_Token*) realloc(tl->tokens, tl->count * sizeof(PGN_Token));

    tl->tokens[tl->count - 1].lexeme = (char*) malloc(strlen(t.lexeme) * sizeof(char) + 1);
    memcpy(tl->tokens[tl->count - 1].lexeme, t.lexeme, strlen(t.lexeme));
    tl->tokens[tl->count - 1].lexeme[strlen(t.lexeme)] = '\0';
    tl->tokens[tl->count - 1].kind = t.kind;
}

void token_print(PGN_Token t)
{
    const char* kind_str;
    switch (t.kind) {
        case PGN_TK_NONE:
            return;
        case PGN_TK_TAG_KEY:
            kind_str = "key";
            break;
        case PGN_TK_TAG_VALUE:
            kind_str = "value";
            break;
        case PGN_TK_MOVE:
            kind_str = "move";
            break;
        case PGN_TK_COMMENT:
            kind_str = "comment";
            break;
        case PGN_TK_GAME_OUTCOME:
            kind_str = "outcome";
            break;
    }
    printf("[TOKEN: %7s] %s", kind_str, t.lexeme);
}

void token_reset(PGN_Token* t)
{
    t->lexeme = NULL;
    t->kind = 0;
}

void consume_while(char** dest, char* buf, size_t size, size_t* i, bool (*filter_func)(char c)) {
    size_t prev_ind = *i;
    size_t word_len = 0;
    while (filter_func(peek(buf, size, *i))) {
        consume(buf, size, i);
        word_len++;
    }
    if (dest != NULL)
        str_n_append(dest, buf + prev_ind, word_len);
}

void parse_lines(char* buf, PGN_TokenList* tl)
{

    bool is_period(char c)         { return c == '.'; }

    bool is_not_quote(char c)      { return c != '"'; }
    bool is_not_space(char c)      { return !isspace(c); }
    bool is_not_right_curly(char c) { return c != '}'; }
    bool is_not_period(char c)     { return c != '.'; }

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

            consume_while(&t.lexeme, buf, size, &i, &is_not_space);
            t.kind = PGN_TK_TAG_KEY;
            token_append(tl, t);
            token_reset(&t);

            consume(buf, size, &i); // Consume ' '
            consume(buf, size, &i); // Consume '"'

            consume_while(&t.lexeme, buf, size, &i, &is_not_quote);
            t.kind = PGN_TK_TAG_VALUE;
            token_append(tl, t);
            token_reset(&t);

            consume(buf, size, &i); // Consume '"'
            consume(buf, size, &i); // Consume the closing square bracket mark
        } else if (c == '{') {
            consume(buf, size, &i); // Consume '{'

            consume_while(&t.lexeme, buf, size, &i, &is_not_right_curly);
            t.kind = PGN_TK_COMMENT;
            token_append(tl, t);
            token_reset(&t);
            consume(buf, size, &i); // Consume '}'
        } else if (is_move_number(buf, size, i)) {
            consume_while(NULL, buf, size, &i, &is_not_period);
            consume_while(NULL, buf, size, &i, &is_period);
        } else if (is_move_text(buf, size, i)) {
            consume_while(&t.lexeme, buf, size, &i, &is_not_space);
            t.kind = PGN_TK_MOVE;
            token_append(tl, t);
            token_reset(&t);
        } else if (isspace(c)) {
            consume(buf, size, &i);
        }
    }
}

PGN_GameResult pgn_parse_result(char* lexeme)
{
    if (!strncmp(lexeme, "1-0", 3))
        return PGN_GR_WHITE_WINS;
    else if (!strncmp(lexeme, "0-1", 3))
        return PGN_GR_BLACK_WINS;
    else if (!strncmp(lexeme, "1/2-1/2", 3))
        return PGN_GR_DRAW;
    else
        // @NOTE: an asterisk result '*' indicates that a
        // game is still ongoing
        return PGN_GR_ONGOING;

}

void pgn_parse(PGN* pgn, const PGN_TokenList tl)
{
    size_t i;
    for (i = 0; i < tl.count; i++) {
        if (tl.tokens[i].kind != PGN_TK_TAG_KEY && tl.tokens[i].kind != PGN_TK_TAG_VALUE)
            break;
    }

    char** dest;
    for (size_t j = 0; j < (i - 1); j += 2) {
        if (!strncmp(tl.tokens[j].lexeme, "result", 6)) {
            pgn->result = pgn_parse_result(tl.tokens[j].lexeme);
            continue;
        }
        if (!strncmp(tl.tokens[j].lexeme, "Event", 5)) {
            dest = &pgn->event;
        } else if (!strncmp(tl.tokens[j].lexeme, "Site", 4)) {
            dest = &pgn->site;
        } else if (!strncmp(tl.tokens[j].lexeme, "Date", 4)) {
            dest = &pgn->date;
        } else if (!strncmp(tl.tokens[j].lexeme, "Round", 5)) {
            dest = &pgn->round;
        } else if (!strncmp(tl.tokens[j].lexeme, "White", 5)) {
            dest = &pgn->white;
        } else if (!strncmp(tl.tokens[j].lexeme, "Black", 5)) {
            dest = &pgn->black;
        } else {
            continue;
        }
        str_append(dest, tl.tokens[j + 1].lexeme);
    }
}

void pgn_print(const PGN* const pgn)
{
    printf("Event: %s\n", pgn->event);
    printf(" Site: %s\n", pgn->site);
    printf(" Date: %s\n", pgn->date);
    printf("Round: %s\n", pgn->round);
    printf("White: %s\n", pgn->white);
    printf("Black: %s\n", pgn->black);
}

int main(void)
{
    const char* file_path = "examples/fischer_v_spassky.pgn";

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

    tl.tokens[tl.count - 1].kind = PGN_TK_GAME_OUTCOME;

#if 0
    for (size_t i = 0; i < tl.count; i++) {
        printf("%3ld. ", i + 1);
        token_print(tl.tokens[i]);
        printf("\n");
    }
#endif

    PGN pgn = { 0 };
    pgn_parse(&pgn, tl);
    pgn_print(&pgn);

    printf("[INFO] Done!\n");
    return 0;
}
