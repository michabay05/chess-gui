#include "board.h"
#include "fen.h"
#include "gui_defs.h"
#include "precalculate.h"
#include "bitboard.h"
#include "move.h"

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

#include "move_gen.h"
int test_main(void) { 
  FENInfo f = parse_fen("r3k2r/pp2bppp/2n2q2/1qPQp3/4P1b1/2N2N2/PPP2PPP/1R2K2R w KQkq - 0 1");
  Board b = {0};
  board_set_from_fen(&b, f);
  board_print(&b);
  MoveList ml = {0};
  movelist_generate(&ml, &b);
  movelist_print_list(ml);
  return 0;
}

void init(void) {
  attack_init();
}

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
