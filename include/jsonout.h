#ifndef LOOGAL_JSONOUT_H
#define LOOGAL_JSONOUT_H

#include <stdio.h>

int loogal_has_flag(int argc, char **argv, const char *flag);
void loogal_json_string(FILE *out, const char *s);
void loogal_json_kv_string(FILE *out, const char *key, const char *value, int comma);
void loogal_json_kv_int(FILE *out, const char *key, long long value, int comma);

#endif
