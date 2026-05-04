#include "rebuild.h"
#include "loogal.h"
#include <stdio.h>
#include <unistd.h>

int cmd_rebuild(int argc, char **argv) {
    if (argc < 1) {
        puts("LOOGAL REBUILD");
        puts("Usage:");
        puts("  loogal rebuild <directories...>");
        loogal_log("rebuild", "error", "missing directories");
        return 1;
    }

    puts("LOOGAL REBUILD");
    puts("────────────────────────");
    puts("Removing old visual memory:");
    puts("  data/loogal.bin");
    puts("  data/records.jsonl");

    unlink("data/loogal.bin");
    unlink("data/records.jsonl");

    ensure_dirs();

    loogal_log("rebuild", "ok", "old index and records removed");

    puts("");
    puts("Reindexing directories...");

    int rc = cmd_index(argc, argv);

    if (rc == 0) {
        puts("");
        puts("Rebuild complete.");
        loogal_log("rebuild", "ok", "rebuild completed");
    } else {
        puts("");
        puts("[ERR] rebuild failed during index phase");
        loogal_log("rebuild", "error", "index phase failed");
    }

    return rc;
}
