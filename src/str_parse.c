#include "str_parse.h"

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

