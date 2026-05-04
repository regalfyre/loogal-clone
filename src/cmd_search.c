#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int pct; LoogalRecord rec; } Hit;
static int cmp_hit(const void *a, const void *b) { return ((Hit*)b)->pct - ((Hit*)a)->pct; }

int cmd_search(int argc, char **argv) {
    if (argc < 1) { loogal_die("search", "usage: loogal search <image> [MIN_PERCENT]"); return 1; }
    int threshold = 60;
    if (argc >= 2) threshold = atoi(argv[1]);
    if (threshold < 0) threshold = 0;
    if (threshold > 100) threshold = 100;

    LoogalImageInfo query;
    if (image_probe(argv[0], &query) != 0) { loogal_die("search", "could not read query image"); return 1; }
    LoogalRecord *records = NULL; size_t count = 0;
    if (read_index_records(&records, &count) != 0) return 1;
    Hit *hits = calloc(count ? count : 1, sizeof(Hit));
    size_t hcount = 0;
    for (size_t i = 0; i < count; i++) {
        int pct = similarity_percent(query.dhash, records[i].dhash);
        if (pct >= threshold) { hits[hcount].pct = pct; hits[hcount].rec = records[i]; hcount++; }
    }
    qsort(hits, hcount, sizeof(Hit), cmp_hit);
    printf("LOOGAL SEARCH\n");
    printf("  Query       : %s\n", argv[0]);
    printf("  Threshold   : %d%% and above\n", threshold);
    printf("  Candidates  : %zu\n", count);
    printf("  Hits        : %zu\n\n", hcount);
    for (size_t i = 0; i < hcount; i++) {
        printf("%3d  %s\n", hits[i].pct, hits[i].rec.path);
    }
    char msg[256]; snprintf(msg, sizeof(msg), "query=%s threshold=%d candidates=%zu hits=%zu", argv[0], threshold, count, hcount);
    loogal_log("search.complete", "ok", msg);
    free(hits); free(records);
    return 0;
}
