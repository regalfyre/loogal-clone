#define _XOPEN_SOURCE 700
#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

static LoogalRecord *g_records = NULL;
static size_t g_count = 0, g_cap = 0;
static FILE *g_jsonl = NULL;

static void add_record(const LoogalImageInfo *info) {
    if (g_count == g_cap) {
        g_cap = g_cap ? g_cap * 2 : 1024;
        g_records = realloc(g_records, g_cap * sizeof(LoogalRecord));
        if (!g_records) exit(2);
    }
    LoogalRecord *r = &g_records[g_count];
    memset(r, 0, sizeof(*r));
    r->id = g_count + 1;
    r->dhash = info->dhash;
    r->aspect = info->aspect;
    r->file_size = info->file_size;
    r->width = info->width;
    r->height = info->height;
    snprintf(r->path, sizeof(r->path), "%s", info->path);

    char *p = json_escape(info->path);
    fprintf(g_jsonl, "{\"id\":%llu,\"tool\":\"loogal\",\"type\":\"image_record\",\"path\":\"%s\",\"extension\":\"%s\",\"width\":%d,\"height\":%d,\"file_size_bytes\":%llu,\"dhash\":\"%016llx\",\"signature_engine\":\"dhash:v1\"}\n",
        (unsigned long long)r->id, p ? p : "", info->ext, info->width, info->height,
        (unsigned long long)info->file_size, (unsigned long long)info->dhash);
    free(p);
    g_count++;
}

static int visitor(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    (void)sb; (void)ftwbuf;
    if (typeflag != FTW_F) return 0;
    if (!image_is_supported(fpath)) return 0;
    LoogalImageInfo info;
    if (image_probe(fpath, &info) == 0) {
        add_record(&info);
        if (g_count % 100 == 0) {
            char msg[128]; snprintf(msg, sizeof(msg), "indexed %zu images so far", g_count);
            loogal_log("index.progress", "ok", msg);
        }
    } else {
        loogal_log("index.skip", "warn", fpath);
    }
    return 0;
}

int cmd_index(int argc, char **argv) {
    if (argc < 1) { loogal_die("index", "usage: loogal index <directories...>"); return 1; }
    ensure_dirs();
    g_jsonl = fopen(LOOGAL_RECORDS_PATH, "w");
    if (!g_jsonl) { loogal_die("index", "could not open records.jsonl"); return 1; }
    loogal_log("index.start", "ok", "starting directory index");
    for (int i = 0; i < argc; i++) {
        char msg[LOOGAL_PATH_MAX + 64]; snprintf(msg, sizeof(msg), "scanning %s", argv[i]);
        loogal_log("index.scan_dir", "ok", msg);
        nftw(argv[i], visitor, 32, FTW_PHYS);
    }
    fclose(g_jsonl);
    if (write_index_records(g_records, g_count) != 0) {
        free(g_records); loogal_die("index", "failed writing binary index"); return 1;
    }
    printf("LOOGAL INDEX COMPLETE\n");
    printf("  Images indexed : %zu\n", g_count);
    printf("  Binary index   : %s\n", LOOGAL_BIN_PATH);
    printf("  Truth records  : %s\n", LOOGAL_RECORDS_PATH);
    printf("  Logs           : %s\n", LOOGAL_LOG_PATH);
    char msg[128]; snprintf(msg, sizeof(msg), "indexed %zu images", g_count);
    loogal_log("index.complete", "ok", msg);
    free(g_records);
    return 0;
}
