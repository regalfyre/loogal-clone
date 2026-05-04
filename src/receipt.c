#include "receipt.h"
#include "pathsafe.h"
#include "loogal.h"
#include <stdio.h>

int loogal_write_move_receipt(
    const LoogalRecord *move,
    const LoogalRecord *keep,
    const char *new_path,
    int similarity_percent
) {
    if (!move || !keep || !new_path) {
        loogal_log("receipt", "error", "invalid receipt arguments");
        return -1;
    }

    char receipt[LOOGAL_PATH_MAX];

    if (loogal_make_receipt_path(receipt, sizeof(receipt), move->path) != 0) {
        loogal_log("receipt", "error", "receipt path too long");
        return -1;
    }

    FILE *r = fopen(receipt, "w");
    if (!r) {
        loogal_log("receipt", "error", "could not create move receipt");
        return -1;
    }

    char ts[64];
    iso_time_now(ts, sizeof(ts));

    fprintf(r,
        "LOOGAL MOVE RECEIPT\n\n"
        "Original:\n"
        "  %s\n\n"
        "Moved to:\n"
        "  %s\n\n"
        "Kept copy:\n"
        "  %s\n\n"
        "Similarity:\n"
        "  %d%%\n\n"
        "Type:\n"
        "  %s\n\n"
        "Dimensions:\n"
        "  %d x %d\n\n"
        "Moved at:\n"
        "  %s\n",
        move->path,
        new_path,
        keep->path,
        similarity_percent,
        file_extension(new_path),
        move->width,
        move->height,
        ts
    );

    fclose(r);
    loogal_log("receipt", "ok", "move receipt written");
    return 0;
}
