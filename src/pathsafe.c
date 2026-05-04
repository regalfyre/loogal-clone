#include <stdio.h>
#include <string.h>
#include <stddef.h>

int loogal_copy_path(char *out, size_t out_size, const char *src) {
    if (!out || !src || out_size == 0) return -1;

    size_t len = strlen(src);
    if (len >= out_size) {
        return -1;
    }

    memcpy(out, src, len + 1);
    return 0;
}

int loogal_make_receipt_path(char *out, size_t out_size, const char *orig_path) {
    if (!out || !orig_path || out_size == 0) return -1;

    int written = snprintf(out, out_size, "%s-loogal.txt", orig_path);

    if (written < 0 || (size_t)written >= out_size) {
        return -1;
    }

    return 0;
}
