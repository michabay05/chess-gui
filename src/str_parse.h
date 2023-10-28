#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define peek(buf, size, ind) peek_ahead(buf, size, ind, 0)
#define peek_next(buf, size, ind) peek_ahead(buf, size, ind, 1)

bool is_at_end(size_t size, size_t ind);
char peek_ahead(char* buf, size_t size, size_t ind, size_t ahead);
char consume(char* buf, size_t size, size_t* ind);
bool str_n_append(char** dest, char* src, size_t size);
bool str_append(char** dest, char* src);
void consume_while(char** dest, char* buf, size_t size, size_t* i, bool (*filter_func)(char c));
