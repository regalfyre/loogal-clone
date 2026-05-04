#define _XOPEN_SOURCE 700
#include "audit.h"
#include "loogal.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void audit_ts(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%S%z", &tmv);
}

void loogal_audit_event(const char *event, const char *status, const char *message) {
    ensure_dirs();

    FILE *f = fopen("data/logs/loogal.jsonl", "a");
    if (!f) return;

    char ts[64];
    audit_ts(ts, sizeof(ts));

    char *e = json_escape(event ? event : "unknown");
    char *s = json_escape(status ? status : "unknown");
    char *m = json_escape(message ? message : "");

    fprintf(f,
        "{\"ts\":\"%s\",\"tool\":\"loogal\",\"schema\":\"loogal.audit.v1\","
        "\"event\":\"%s\",\"status\":\"%s\",\"message\":\"%s\"}\n",
        ts,
        e ? e : "",
        s ? s : "",
        m ? m : ""
    );

    if (e) free(e);
    if (s) free(s);
    if (m) free(m);

    fclose(f);
}

void loogal_log(const char *event, const char *status, const char *message) {
    loogal_audit_event(event, status, message);
}
