#include "jsonout.h"
#include <string.h>

int loogal_has_flag(int argc, char **argv, const char *flag) {
    if (!flag) return 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i] && strcmp(argv[i], flag) == 0) return 1;
    }
    return 0;
}

void loogal_json_string(FILE *out, const char *s) {
    fputc('"', out);

    if (s) {
        for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
            switch (*p) {
                case '"':  fputs("\\\"", out); break;
                case '\\': fputs("\\\\", out); break;
                case '\n': fputs("\\n", out); break;
                case '\r': fputs("\\r", out); break;
                case '\t': fputs("\\t", out); break;
                default:
                    if (*p < 32) {
                        fprintf(out, "\\u%04x", *p);
                    } else {
                        fputc(*p, out);
                    }
            }
        }
    }

    fputc('"', out);
}

void loogal_json_kv_string(FILE *out, const char *key, const char *value, int comma) {
    loogal_json_string(out, key);
    fputs(": ", out);
    loogal_json_string(out, value);
    if (comma) fputc(',', out);
    fputc('\n', out);
}

void loogal_json_kv_int(FILE *out, const char *key, long long value, int comma) {
    loogal_json_string(out, key);
    fprintf(out, ": %lld", value);
    if (comma) fputc(',', out);
    fputc('\n', out);
}
