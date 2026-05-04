#include "learn.h"
#include "loogal.h"
#include <stdio.h>

int cmd_learn(int argc, char **argv) {
    if (argc < 1) {
        puts("LOOGAL LEARN");
        puts("Usage:");
        puts("  loogal learn <directories...>");
        puts("");
        puts("Meaning:");
        puts("  Teach Loogal where to remember images.");
        loogal_log("learn", "error", "missing directories");
        return 1;
    }

    puts("LOOGAL LEARN");
    puts("────────────────────────");
    puts("Teaching visual memory from directories...");
    puts("");

    loogal_log("learn", "ok", "delegating to index");

    return cmd_index(argc, argv);
}
