#include "config.h"
#include "jsonout.h"
#include "loogal.h"
#include <stdio.h>

#define LOOGAL_CONFIG_PATH "data/config.json"

static void write_default_config(void) {
    FILE *f = fopen(LOOGAL_CONFIG_PATH, "w");
    if (!f) {
        loogal_log("config", "error", "could not write data/config.json");
        return;
    }

    fputs("{\n", f);
    fputs("  \"tool\": \"loogal\",\n", f);
    fputs("  \"schema\": \"loogal.config.v1\",\n", f);
    fputs("  \"binary_index\": \"data/loogal.bin\",\n", f);
    fputs("  \"records\": \"data/records.jsonl\",\n", f);
    fputs("  \"logs\": \"data/logs/loogal.jsonl\",\n", f);
    fputs("  \"manifests\": \"data/manifests/\",\n", f);
    fputs("  \"default_threshold_percent\": 60,\n", f);
    fputs("  \"signature_engine\": \"dHash v1\",\n", f);
    fputs("  \"hot_path\": \"binary-only\"\n", f);
    fputs("}\n", f);

    fclose(f);
    loogal_log("config", "ok", "default config written");
}

int cmd_config(int argc, char **argv) {
    ensure_dirs();

    FILE *f = fopen(LOOGAL_CONFIG_PATH, "r");
    if (!f) {
        write_default_config();
        f = fopen(LOOGAL_CONFIG_PATH, "r");
    }

    if (!f) {
        puts("LOOGAL CONFIG");
        puts("  [ERR] could not create or read data/config.json");
        loogal_log("config", "error", "config unavailable");
        return 1;
    }

    int as_json = loogal_has_flag(argc, argv, "--json");

    if (as_json) {
        int c;
        while ((c = fgetc(f)) != EOF) putchar(c);
    } else {
        puts("LOOGAL CONFIG");
        puts("────────────────────────");
        puts("Path:");
        puts("  data/config.json");
        puts("");
        puts("Defaults:");
        puts("  default threshold : 60%");
        puts("  signature engine  : dHash v1");
        puts("  hot path          : binary-only");
        puts("");
        puts("Use:");
        puts("  loogal config --json");
    }

    fclose(f);
    loogal_log("config", "ok", as_json ? "printed config json" : "printed config summary");
    return 0;
}
