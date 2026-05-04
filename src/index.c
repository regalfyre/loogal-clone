#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_index_records(LoogalRecord *records, size_t count) {
    ensure_dirs();
    FILE *f = fopen(LOOGAL_BIN_PATH, "wb");
    if (!f) { loogal_die("index", "could not open binary index for writing"); return -1; }
    LoogalHeader h;
    memcpy(h.magic, LOOGAL_MAGIC, 8);
    h.version = 1;
    h.count = count;
    h.created_unix = (uint64_t)now_unix();
    if (fwrite(&h, sizeof(h), 1, f) != 1) { fclose(f); return -1; }
    if (count && fwrite(records, sizeof(LoogalRecord), count, f) != count) { fclose(f); return -1; }
    fclose(f);
    return 0;
}

int read_index_records(LoogalRecord **records, size_t *count) {
    *records = NULL; *count = 0;
    FILE *f = fopen(LOOGAL_BIN_PATH, "rb");
    if (!f) { loogal_die("search", "binary index not found; run loogal index first"); return -1; }
    LoogalHeader h;
    if (fread(&h, sizeof(h), 1, f) != 1 || memcmp(h.magic, LOOGAL_MAGIC, 8) != 0) {
        fclose(f); loogal_die("search", "invalid loogal binary index"); return -1;
    }
    LoogalRecord *r = calloc(h.count, sizeof(LoogalRecord));
    if (!r && h.count) { fclose(f); return -1; }
    if (h.count && fread(r, sizeof(LoogalRecord), h.count, f) != h.count) {
        free(r); fclose(f); return -1;
    }
    fclose(f);
    *records = r; *count = (size_t)h.count;
    return 0;
}
