#include "verify.h"
#include "jsonout.h"
#include "loogal.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static long count_lines(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    long lines = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') lines++;
    }

    fclose(f);
    return lines;
}

static long verify_file_size_bytes(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

int cmd_verify(int argc, char **argv) {
    int as_json = loogal_has_flag(argc, argv, "--json");

    long records = count_lines("data/records.jsonl");
    long bin_size = verify_file_size_bytes("data/loogal.bin");

    long expected_bin = -1;
    int ok = 1;

    if (records < 0) ok = 0;
    if (bin_size < 0) ok = 0;

    if (records >= 0) {
        expected_bin = 28 + records * (long)sizeof(LoogalRecord);
        if (bin_size != expected_bin) ok = 0;
    }

    if (as_json) {
        puts("{");
        printf("  ");
        loogal_json_kv_string(stdout, "status", ok ? "ok" : "error", 1);
        printf("  ");
        loogal_json_kv_int(stdout, "records_jsonl_count", records, 1);
        printf("  ");
        loogal_json_kv_int(stdout, "binary_size_bytes", bin_size, 1);
        printf("  ");
        loogal_json_kv_int(stdout, "index_header_bytes", 28, 1);
        printf("  ");
        loogal_json_kv_int(stdout, "expected_binary_size_bytes", expected_bin, 0);
        puts("}");
    } else {
        puts("LOOGAL VERIFY");
        puts("────────────────────────");

        if (records >= 0)
            printf("[OK] records.jsonl lines : %ld\n", records);
        else
            puts("[ERR] missing data/records.jsonl");

        if (bin_size >= 0)
            printf("[OK] loogal.bin bytes    : %ld\n", bin_size);
        else
            puts("[ERR] missing data/loogal.bin");

        if (records >= 0 && bin_size >= 0) {
            printf("[INFO] expected bin bytes: %ld\n", expected_bin);

            if (bin_size == expected_bin)
                puts("[OK] binary index matches record count");
            else
                puts("[ERR] binary index does not match record count");
        }

        puts("");
        printf("System integrity: %s\n", ok ? "OK" : "ERROR");
    }

    loogal_log("verify", ok ? "ok" : "error", ok ? "index and records agree" : "index and records mismatch");
    return ok ? 0 : 1;
}
