#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "precalculate.h"
#include "gui_defs.h"
#include "pgn.h"

typedef enum { GUI, TERM, DEBUG } Mode;

void parse_cmd_args(int argc, char **argv, Mode* mode) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "GUI"))
            *mode = GUI;
        else if (!strcmp(argv[1], "Term"))
            *mode = TERM;
        else if (!strcmp(argv[1], "Debug"))
            *mode = DEBUG;
    }
}

void init(void) { attack_init(); }

int main(int argc, char *argv[]) {
    init();
    // GUI is the default mode
    Mode mode = DEBUG;
    parse_cmd_args(argc, argv, &mode);
    switch (mode) {
    case GUI:
        gui_main();
        break;
    case TERM:
        printf("********** TODO: Terminal mode is still not implemented.\n");
        return 1;
        // TODO: Add term main();
    case DEBUG:
        return pgn_main();
        // return 0;
    };
    return 0;
}
