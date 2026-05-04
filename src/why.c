#include "why.h"
#include "jsonout.h"
#include "loogal.h"
#include <stdio.h>
#include <string.h>

static int line_has_path(const char *line, const char *path) {
    return line && path && strstr(line, path) != NULL;
}

int cmd_why(int argc, char **argv) {
    if (argc < 1) {
        puts("LOOGAL WHY");
        puts("Usage:");
        puts("  loogal why <path> [--json]");
        loogal_log("why", "error", "missing path");
        return 1;
    }

    const char *target = argv[0];
    int as_json = loogal_has_flag(argc, argv, "--json");

    FILE *f = fopen("data/records.jsonl", "r");
    if (!f) {
        puts("LOOGAL WHY");
        puts("[ERR] missing data/records.jsonl");
        puts("Run:");
        puts("  loogal index <directories...>");
        loogal_log("why", "error", "records file missing");
        return 1;
    }

    char line[4096];
    char found[4096];
    found[0] = '\0';

    while (fgets(line, sizeof(line), f)) {
        if (line_has_path(line, target)) {
            snprintf(found, sizeof(found), "%s", line);
            break;
        }
    }

    fclose(f);

    if (found[0] == '\0') {
        if (as_json) {
            puts("{");
            printf("  ");
            loogal_json_kv_string(stdout, "status", "not_found", 1);
            printf("  ");
            loogal_json_kv_string(stdout, "path", target, 0);
            puts("}");
        } else {
            puts("LOOGAL WHY");
            puts("────────────────────────");
            printf("Path: %s\n", target);
            puts("Status: not found in Loogal records");
            puts("");
            puts("Try:");
            puts("  loogal index <directory-containing-this-file>");
        }

        loogal_log("why", "miss", target);
        return 1;
    }

    if (as_json) {
        puts("{");
        printf("  ");
        loogal_json_kv_string(stdout, "status", "found", 1);
        printf("  ");
        loogal_json_kv_string(stdout, "path", target, 1);
        printf("  ");
        loogal_json_kv_string(stdout, "record_jsonl", found, 0);
        puts("}");
    } else {
        puts("LOOGAL WHY");
        puts("────────────────────────");
        printf("Path: %s\n", target);
        puts("Status: found in visual memory");
        puts("");
        puts("Known because:");
        puts("  indexed into data/records.jsonl");
        puts("  represented in data/loogal.bin");
        puts("  searchable by dHash v1 appearance signature");
        puts("");
        puts("Raw record:");
        fputs(found, stdout);
    }

    loogal_log("why", "ok", target);
    return 0;
}
