#define _XOPEN_SOURCE 700
#include "loogal.h"
#include "jsonout.h"
#include "similar.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef SIMILAR_CMD_MAX
#define SIMILAR_CMD_MAX 8192
#endif

#ifndef SIMILAR_OUTPUT_MAX
#define SIMILAR_OUTPUT_MAX 262144
#endif

typedef struct {
    const char *path;
    const char *place;
    const char *min_percent;
    const char *limit;
    const char *offset;
    int memory;
    int as_json;
    int dry_run;
    int push_history;
} SimilarArgs;

static void similar_usage(void) {
    puts("LOOGAL SIMILAR — create a new search session from an image path");
    puts("");
    puts("Commands:");
    puts("  loogal similar --path <image> --place <dir> [--min N] [--limit N] [--json]");
    puts("  loogal similar --path <image> --memory [--limit N] [--json]");
    puts("  loogal similar --path <image> --place <dir> --push-history --json");
    puts("");
    puts("Purpose:");
    puts("  This is the future loogal-window Search Similar button.");
    puts("  It reuses loogal-session and loogal-history instead of duplicating search logic.");
}

static const char *arg_value(int argc, char **argv, const char *flag) {
    for (int i = 0; i < argc - 1; i++) {
        if (strcmp(argv[i], flag) == 0) return argv[i + 1];
    }
    return NULL;
}

static int has_flag_local(int argc, char **argv, const char *flag) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) return 1;
    }
    return 0;
}

static int shell_quote(const char *in, char *out, size_t out_sz) {
    if (!in || !out || out_sz < 3) return -1;
    size_t j = 0;
    out[j++] = '\'';
    for (size_t i = 0; in[i]; i++) {
        if (in[i] == '\'') {
            const char *esc = "'\\''";
            for (size_t k = 0; esc[k]; k++) {
                if (j + 1 >= out_sz) return -1;
                out[j++] = esc[k];
            }
        } else {
            if (j + 1 >= out_sz) return -1;
            out[j++] = in[i];
        }
    }
    if (j + 1 >= out_sz) return -1;
    out[j++] = '\'';
    out[j] = 0;
    return 0;
}

static int read_command_output(const char *cmd, char *out, size_t out_sz, int *exit_code) {
    if (!cmd || !out || out_sz == 0) return -1;
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    size_t used = 0;
    out[0] = 0;
    while (!feof(fp)) {
        if (used + 1 >= out_sz) {
            pclose(fp);
            return -2;
        }
        size_t n = fread(out + used, 1, out_sz - used - 1, fp);
        used += n;
        out[used] = 0;
        if (ferror(fp)) {
            pclose(fp);
            return -1;
        }
    }
    int st = pclose(fp);
    if (exit_code) {
        if (WIFEXITED(st)) *exit_code = WEXITSTATUS(st);
        else *exit_code = 127;
    }
    return 0;
}

static int json_extract_string(const char *json, const char *key, char *out, size_t out_sz) {
    if (!json || !key || !out || out_sz == 0) return -1;
    char pattern[128];
    int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    if (pn < 0 || pn >= (int)sizeof(pattern)) return -1;
    const char *p = strstr(json, pattern);
    if (!p) return -1;
    p = strchr(p + strlen(pattern), ':');
    if (!p) return -1;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p != '"') return -1;
    p++;
    size_t j = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && p[1]) {
            p++;
        }
        if (j + 1 >= out_sz) return -1;
        out[j++] = *p++;
    }
    out[j] = 0;
    return j > 0 ? 0 : -1;
}

static int parse_args(int argc, char **argv, SimilarArgs *a) {
    memset(a, 0, sizeof(*a));
    a->path = arg_value(argc, argv, "--path");
    if (!a->path) a->path = arg_value(argc, argv, "--query");
    if (!a->path && argc >= 1 && argv[0][0] != '-') a->path = argv[0];
    a->place = arg_value(argc, argv, "--place");
    a->min_percent = arg_value(argc, argv, "--min");
    a->limit = arg_value(argc, argv, "--limit");
    a->offset = arg_value(argc, argv, "--offset");
    a->memory = has_flag_local(argc, argv, "--memory");
    a->as_json = has_flag_local(argc, argv, "--json");
    a->dry_run = has_flag_local(argc, argv, "--dry-run");
    a->push_history = has_flag_local(argc, argv, "--push-history");
    if (!a->min_percent) a->min_percent = "60";
    if (!a->limit) a->limit = "50";
    if (!a->offset) a->offset = "0";
    if (!a->path) return -1;
    if (!a->memory && !a->place) return -1;
    return 0;
}

static int build_session_command(const SimilarArgs *a, char *cmd, size_t cmd_sz) {
    char q_path[LOOGAL_PATH_MAX * 2];
    char q_place[LOOGAL_PATH_MAX * 2];
    if (shell_quote(a->path, q_path, sizeof(q_path)) != 0) return -1;
    if (a->place && shell_quote(a->place, q_place, sizeof(q_place)) != 0) return -1;

    int n;
    if (a->memory) {
        n = snprintf(cmd, cmd_sz,
                     "./loogal session create --query %s --memory --min %s --limit %s --offset %s --json",
                     q_path, a->min_percent, a->limit, a->offset);
    } else {
        n = snprintf(cmd, cmd_sz,
                     "./loogal session create --query %s --place %s --min %s --limit %s --offset %s --json",
                     q_path, q_place, a->min_percent, a->limit, a->offset);
    }
    return (n < 0 || n >= (int)cmd_sz) ? -1 : 0;
}

static int build_history_command(const SimilarArgs *a, const char *session_id, char *cmd, size_t cmd_sz) {
    char q_path[LOOGAL_PATH_MAX * 2];
    char q_session[LOOGAL_PATH_MAX * 2];
    if (shell_quote(a->path, q_path, sizeof(q_path)) != 0) return -1;
    if (shell_quote(session_id, q_session, sizeof(q_session)) != 0) return -1;
    int n = snprintf(cmd, cmd_sz,
                     "./loogal history push --query %s --session %s --offset 0 --selected 1 --json",
                     q_path, q_session);
    return (n < 0 || n >= (int)cmd_sz) ? -1 : 0;
}

int cmd_similar(int argc, char **argv) {
    if (argc < 1 || strcmp(argv[0], "help") == 0 || strcmp(argv[0], "--help") == 0) {
        similar_usage();
        return 0;
    }

    SimilarArgs a;
    if (parse_args(argc, argv, &a) != 0) {
        similar_usage();
        loogal_log("similar", "error", "missing --path and scope");
        return 1;
    }

    char session_cmd[SIMILAR_CMD_MAX];
    if (build_session_command(&a, session_cmd, sizeof(session_cmd)) != 0) {
        fprintf(stderr, "[loogal:similar_error] failed to build session command\n");
        loogal_log("similar", "error", "failed to build session command");
        return 1;
    }

    if (a.dry_run) {
        if (a.as_json) {
            puts("{");
            printf("  "); loogal_json_kv_string(stdout, "tool", "loogal", 1);
            printf("  "); loogal_json_kv_string(stdout, "type", "similar.dry_run", 1);
            printf("  "); loogal_json_kv_string(stdout, "path", a.path, 1);
            printf("  "); loogal_json_kv_string(stdout, "place", a.memory ? "memory" : a.place, 1);
            printf("  "); loogal_json_kv_string(stdout, "session_command", session_cmd, 0);
            puts("}");
        } else {
            printf("[loogal:similar_dry_run] %s\n", session_cmd);
        }
        loogal_log("similar", "ok", "dry-run similar command");
        return 0;
    }

    char session_json[SIMILAR_OUTPUT_MAX];
    int session_exit = 0;
    int r = read_command_output(session_cmd, session_json, sizeof(session_json), &session_exit);
    if (r != 0 || session_exit != 0) {
        fprintf(stderr, "[loogal:similar_error] session create failed exit=%d read=%d\n", session_exit, r);
        loogal_log("similar", "error", "session create failed");
        return 1;
    }

    char session_id[256] = {0};
    char meta_path[LOOGAL_PATH_MAX] = {0};
    char results_path[LOOGAL_PATH_MAX] = {0};
    if (json_extract_string(session_json, "id", session_id, sizeof(session_id)) != 0) {
        fprintf(stderr, "[loogal:similar_error] could not extract session id\n");
        loogal_log("similar", "error", "could not extract session id");
        return 1;
    }
    json_extract_string(session_json, "meta", meta_path, sizeof(meta_path));
    json_extract_string(session_json, "results", results_path, sizeof(results_path));

    int history_exit = -1;
    char history_cmd[SIMILAR_CMD_MAX] = {0};
    char history_json[SIMILAR_OUTPUT_MAX] = {0};
    if (a.push_history) {
        if (build_history_command(&a, session_id, history_cmd, sizeof(history_cmd)) != 0) {
            fprintf(stderr, "[loogal:similar_error] failed to build history command\n");
            loogal_log("similar", "error", "failed to build history command");
            return 1;
        }
        r = read_command_output(history_cmd, history_json, sizeof(history_json), &history_exit);
        if (r != 0 || history_exit != 0) {
            fprintf(stderr, "[loogal:similar_error] history push failed exit=%d read=%d\n", history_exit, r);
            loogal_log("similar", "error", "history push failed");
            return 1;
        }
    }

    loogal_log("similar", "ok", "created similar session");

    if (a.as_json) {
        puts("{");
        printf("  "); loogal_json_kv_string(stdout, "tool", "loogal", 1);
        printf("  "); loogal_json_kv_string(stdout, "type", "similar.created", 1);
        printf("  "); loogal_json_kv_string(stdout, "path", a.path, 1);
        printf("  "); loogal_json_kv_string(stdout, "scope", a.memory ? "memory" : a.place, 1);
        printf("  "); loogal_json_kv_string(stdout, "session_id", session_id, 1);
        printf("  "); loogal_json_kv_string(stdout, "meta", meta_path, 1);
        printf("  "); loogal_json_kv_string(stdout, "results", results_path, 1);
        printf("  \"history_pushed\": %s,\n", a.push_history ? "true" : "false");
        printf("  "); loogal_json_kv_string(stdout, "session_command", session_cmd, a.push_history ? 1 : 0);
        if (a.push_history) {
            printf("  "); loogal_json_kv_string(stdout, "history_command", history_cmd, 0);
        }
        puts("}");
    } else {
        printf("[loogal:similar_ok] session=%s path=%s\n", session_id, a.path);
        if (meta_path[0]) printf("  meta:    %s\n", meta_path);
        if (results_path[0]) printf("  results: %s\n", results_path);
        if (a.push_history) printf("  history: pushed\n");
    }

    return 0;
}
