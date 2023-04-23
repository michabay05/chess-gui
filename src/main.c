#include "defs.h"
#include "fen.h"
#include "gui_defs.h"

#include <string.h>

typedef enum {
  GUI,
  TERM,
  DEBUG
} Mode;

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

int test_main(void) {
  FENInfo fen = parse_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
  fen_info_print(&fen);
  return 0;
}

int main(int argc, char *argv[]) {
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
