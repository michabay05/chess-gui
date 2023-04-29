#include "bitboard.h"
#include "board.h"
#include "fen.h"
#include "gui_defs.h"
#include "move.h"
#include "precalculate.h"

#include <string.h>

typedef enum { GUI, TERM, DEBUG } Mode;

Mode parse_cmd_args(int argc, char **argv) {
  Mode selected = GUI; // GUI is the default mode
  if (argc >= 2) {
    if (!strcmp(argv[1], "Term"))
      selected = TERM;
    else if (!strcmp(argv[1], "Debug"))
      selected = DEBUG;
  }
  return selected;
}

int test_main(void) { return 0; }

void init(void) { attack_init(); }

int main(int argc, char *argv[]) {
  init();
  switch (parse_cmd_args(argc, argv)) {
  case GUI:
    gui_main();
    break;
  case TERM:
    printf("********** TODO: Terminal mode is still not implemented.\n");
    return 1;
    // TODO: Add term main();
  case DEBUG:
    return test_main();
  };
  return 0;
}
