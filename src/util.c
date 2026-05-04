#define _XOPEN_SOURCE 700
#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

static void mkdir_if_missing(const char *p) {
    if (mkdir(p, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "LOOGAL ERROR: failed to create %s: %s\n", p, strerror(errno));
    }
}

int ensure_dirs(void) {
    mkdir_if_missing("data");
    mkdir_if_missing("data/logs");
    mkdir_if_missing("data/manifests");
    return 0;
}

long now_unix(void) { return (long)time(NULL); }

void iso_time_now(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%S%z", &tmv);
}

char *json_escape(const char *s) {
    size_t len = strlen(s), cap = len * 2 + 16, j = 0;
    char *out = malloc(cap);
    if (!out) return NULL;
    for (size_t i = 0; i < len; i++) {
        if (j + 8 >= cap) { cap *= 2; out = realloc(out, cap); if (!out) return NULL; }
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') { out[j++] = '\\'; out[j++] = c; }
        else if (c == '\n') { out[j++] = '\\'; out[j++] = 'n'; }
        else if (c == '\r') { out[j++] = '\\'; out[j++] = 'r'; }
        else if (c == '\t') { out[j++] = '\\'; out[j++] = 't'; }
        else if (c < 32) { j += snprintf(out + j, cap - j, "\\u%04x", c); }
        else out[j++] = c;
    }
    out[j] = 0;
    return out;
}



void loogal_die(const char *event, const char *message) {
    loogal_log(event, "error", message);
    fprintf(stderr, "LOOGAL ERROR: %s\n", message);
}

uint64_t file_size_bytes(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (uint64_t)st.st_size;
}

const char *file_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return dot ? dot + 1 : "";
}
