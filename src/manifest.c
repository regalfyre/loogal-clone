#include "manifest.h"
#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>

int loogal_manifest_write_move(
    FILE *manifest,
    const LoogalRecord *move,
    const LoogalRecord *keep,
    const char *new_path,
    const char *filename,
    int similarity_percent
) {
    if (!manifest || !move || !keep || !new_path || !filename) {
        loogal_log("manifest", "error", "invalid manifest move arguments");
        return -1;
    }

    char *op = json_escape(move->path);
    char *np = json_escape(new_path);
    char *kp = json_escape(keep->path);
    char *fn = json_escape(filename);
    char *ext = json_escape(file_extension(new_path));

    fprintf(
        manifest,
        "    {"
        "\"original_path\":\"%s\","
        "\"new_path\":\"%s\","
        "\"kept_path\":\"%s\","
        "\"filename\":\"%s\","
        "\"extension\":\"%s\","
        "\"dimensions\":{\"width\":%d,\"height\":%d},"
        "\"file_size_bytes\":%llu,"
        "\"similarity_percent\":%d,"
        "\"reason\":\"duplicate_of_kept\""
        "}",
        op ? op : "",
        np ? np : "",
        kp ? kp : "",
        fn ? fn : "",
        ext ? ext : "",
        move->width,
        move->height,
        (unsigned long long)move->file_size,
        similarity_percent
    );

    free(op);
    free(np);
    free(kp);
    free(fn);
    free(ext);

    loogal_log("manifest", "ok", "move written to manifest");
    return 0;
}
