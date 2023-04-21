#include "raylib.h"

#include "defs.h"
#include "gui_defs.h"

#include <string.h>

typedef enum {
	GUI,
	TERM,
} Mode;

Mode parse_cmd_args(int argc, char** argv) {
	Mode selected = GUI; // GUI is the default mode
	if (argc >= 2) {
		if (!strcmp(argv[1], "term")) selected = TERM;
	}
	return selected;
}

int main(int argc, char* argv[]) {
	switch (parse_cmd_args(argc, argv)) {
		case GUI:
			gui_main();
			break;
		case TERM:
			return 1;
			// TODO: Add term main();
	};
	return 0;
}
