#include "loogal.h"
#include "jsonout.h"
#include <stdio.h>
#include <stdlib.h>

int cmd_stats(int argc, char **argv) {
    int as_json = loogal_has_flag(argc, argv, "--json");

    (void)argc; (void)argv;
    LoogalRecord *records = NULL; size_t count = 0;
    if (read_index_records(&records, &count) != 0) return 1;
    size_t near_dupes = 0;
    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            if (similarity_percent(records[i].dhash, records[j].dhash) >= 90) { near_dupes++; break; }
        }
    }
    if (as_json) {
puts("{");
printf("  ");
loogal_json_kv_string(stdout, "status", "ok", 1);
printf("  ");
loogal_json_kv_int(stdout, "total_images_indexed", (long long)count, 1);
printf("  ");
loogal_json_kv_int(stdout, "redundant_near_duplicate", (long long)near_dupes, 1);
printf("  ");
loogal_json_kv_string(stdout, "signature_engine", "dHash v1", 1);
printf("  ");
loogal_json_kv_string(stdout, "comparison_mode", "deterministic", 1);
printf("  ");
loogal_json_kv_string(stdout, "storage_format", "binary packed", 1);
printf("  ");
loogal_json_kv_string(stdout, "search_execution", "no JSON in hot path", 0);
puts("}");
loogal_log("stats", "ok", "printed status json");
free(records);
return 0;
}

puts("LOOGAL — VISUAL MEMORY STATUS");
    puts("────────────────────────────────────────");
    puts("");
    puts("Corpus");
    printf("  Total images indexed        : %zu\n", count);
    printf("  Redundant / near-duplicate  : %zu\n", near_dupes);
    puts("");
    puts("Perceptual Model");
    puts("  Signature engine            : dHash v1");
    puts("  Comparison mode             : deterministic");
    puts("");
    puts("Index Engine");
    puts("  Storage format              : binary packed");
    puts("  Search execution            : no JSON in hot path");
    puts("  Recall readiness            : immediate");
    puts("");
    puts("Interpretation");
    puts("  Images can now be recalled by appearance,");
    puts("  independent of filename or location.");
    loogal_log("stats", "ok", "printed status");
    free(records);
    return 0;
}
