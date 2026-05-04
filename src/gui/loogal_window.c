#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gui/loogal_window.h"

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *list_box;
    GtkWidget *status_label;
    GtkWidget *back_button;
    GtkWidget *forward_button;
    GtkWidget *next_button;
    GtkWidget *prev_button;

    char session_id[256];
    int offset;
    int limit;
    int history_index;
    int history_count;
    char history[128][256];
} AppState;

static int shell_quote(const char *in, char *out, size_t out_sz) {
    size_t j = 0;
    if (!in || !out || out_sz < 3) return -1;
    if (j + 1 >= out_sz) return -1;
    out[j++] = '\'';
    for (size_t i = 0; in[i]; i++) {
        if (in[i] == '\'') {
            if (j + 4 >= out_sz) return -1;
            out[j++] = '\'';
            out[j++] = '\\';
            out[j++] = '\'';
            out[j++] = '\'';
        } else {
            if (j + 1 >= out_sz) return -1;
            out[j++] = in[i];
        }
    }
    if (j + 2 >= out_sz) return -1;
    out[j++] = '\'';
    out[j] = 0;
    return 0;
}

static char *run_command_capture(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    size_t cap = 8192;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        pclose(fp);
        return NULL;
    }
    buf[0] = 0;

    char chunk[1024];
    while (fgets(chunk, sizeof(chunk), fp)) {
        size_t n = strlen(chunk);
        if (len + n + 1 > cap) {
            cap = (len + n + 1) * 2;
            char *nb = realloc(buf, cap);
            if (!nb) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            buf = nb;
        }
        memcpy(buf + len, chunk, n);
        len += n;
        buf[len] = 0;
    }

    int rc = pclose(fp);
    (void)rc;
    return buf;
}

static int json_get_string_after(const char *start, const char *key, char *out, size_t out_sz) {
    const char *p = strstr(start, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '"') return 0;
    p++;

    size_t j = 0;
    while (*p && *p != '"' && j + 1 < out_sz) {
        if (*p == '\\' && p[1]) p++;
        out[j++] = *p++;
    }
    out[j] = 0;
    return j > 0;
}

static int json_get_int_after(const char *start, const char *key, int *out) {
    const char *p = strstr(start, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    *out = atoi(p);
    return 1;
}

static int json_get_ll_after(const char *start, const char *key, long long *out) {
    const char *p = strstr(start, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    *out = atoll(p);
    return 1;
}

static int json_get_double_after(const char *start, const char *key, double *out) {
    const char *p = strstr(start, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    *out = atof(p);
    return 1;
}

static int parse_page_json(const char *json, LoogalWindowPage *page) {
    memset(page, 0, sizeof(*page));
    page->limit = LOOGAL_WINDOW_PAGE_LIMIT;

    json_get_int_after(json, "\"offset\"", &page->offset);
    json_get_int_after(json, "\"limit\"", &page->limit);
    json_get_int_after(json, "\"returned\"", &page->returned);
    json_get_int_after(json, "\"total_hits\"", &page->total_hits);

    const char *results = strstr(json, "\"results\"");
    if (!results) return 0;

    int count = 0;
    const char *p = results;
    while ((p = strstr(p, "\"rank\"")) && count < LOOGAL_WINDOW_MAX_RESULTS) {
        LoogalWindowResult *r = &page->results[count];

        json_get_int_after(p, "\"rank\"", &r->rank);
        json_get_string_after(p, "\"path\"", r->path, sizeof(r->path));
        json_get_string_after(p, "\"label\"", r->label, sizeof(r->label));
        json_get_double_after(p, "\"similarity_percent\"", &r->similarity_percent);
        json_get_int_after(p, "\"hash_distance\"", &r->hash_distance);
        json_get_int_after(p, "\"width\"", &r->width);
        json_get_int_after(p, "\"height\"", &r->height);
        json_get_ll_after(p, "\"file_size_bytes\"", &r->file_size_bytes);

        if (r->path[0]) count++;
        p += 6;
    }

    page->returned = count;
    return count;
}

static void set_status(AppState *st, const char *msg) {
    if (st && st->status_label) gtk_label_set_text(GTK_LABEL(st->status_label), msg);
}

static void run_action_command(AppState *st, const char *verb, const char *path) {
    char quoted[4096];
    char cmd[8192];

    if (shell_quote(path, quoted, sizeof(quoted)) != 0) {
        set_status(st, "path too long for action");
        return;
    }

    int n = snprintf(cmd, sizeof(cmd), "./loogal action %s --path %s", verb, quoted);
    if (n < 0 || n >= (int)sizeof(cmd)) {
        set_status(st, "action command too long");
        return;
    }

    char *out = run_command_capture(cmd);
    if (out) {
        set_status(st, out);
        free(out);
    } else {
        set_status(st, "action failed");
    }
}

static void clear_list(GtkWidget *box) {
    GtkWidget *child = gtk_widget_get_first_child(box);
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(box), child);
        child = next;
    }
}

static void load_page(AppState *st);

static void history_push(AppState *st, const char *session_id) {
    if (!session_id || !session_id[0]) return;

    if (st->history_index >= 0 && st->history_index < st->history_count - 1) {
        st->history_count = st->history_index + 1;
    }

    if (st->history_count >= 128) {
        memmove(st->history, st->history + 1, sizeof(st->history[0]) * 127);
        st->history_count = 127;
        st->history_index = 126;
    }

    snprintf(st->history[st->history_count], sizeof(st->history[0]), "%s", session_id);
    st->history_count++;
    st->history_index = st->history_count - 1;
}

static void on_reveal_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    (void)button;
    const char *path = g_object_get_data(G_OBJECT(button), "loogal-path");
    AppState *st = g_object_get_data(G_OBJECT(button), "loogal-state");
    if (path) run_action_command(st, "reveal", path);
}

static void on_open_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    (void)button;
    const char *path = g_object_get_data(G_OBJECT(button), "loogal-path");
    AppState *st = g_object_get_data(G_OBJECT(button), "loogal-state");
    if (path) run_action_command(st, "open", path);
}

static void on_copy_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    (void)button;
    const char *path = g_object_get_data(G_OBJECT(button), "loogal-path");
    AppState *st = g_object_get_data(G_OBJECT(button), "loogal-state");
    if (path) run_action_command(st, "copy-path", path);
}

static void on_similar_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    (void)button;
    const char *path = g_object_get_data(G_OBJECT(button), "loogal-path");
    AppState *st = g_object_get_data(G_OBJECT(button), "loogal-state");
    if (!path || !st) return;

    char quoted[4096];
    char cmd[8192];
    if (shell_quote(path, quoted, sizeof(quoted)) != 0) {
        set_status(st, "path too long for similar search");
        return;
    }

    int n = snprintf(cmd, sizeof(cmd),
        "./loogal window-api similar --path %s --place \"$HOME/Pictures\" --push-history --json",
        quoted);
    if (n < 0 || n >= (int)sizeof(cmd)) {
        set_status(st, "similar command too long");
        return;
    }

    char *out = run_command_capture(cmd);
    if (!out) {
        set_status(st, "similar search failed");
        return;
    }

    char new_session[256] = {0};
    if (json_get_string_after(out, "\"session_id\"", new_session, sizeof(new_session))) {
        snprintf(st->session_id, sizeof(st->session_id), "%s", new_session);
        st->offset = 0;
        history_push(st, new_session);
        free(out);
        load_page(st);
        return;
    }

    set_status(st, out);
    free(out);
}

static GtkWidget *make_result_row(AppState *st, const LoogalWindowResult *r) {
    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(outer, 8);
    gtk_widget_set_margin_bottom(outer, 8);
    gtk_widget_set_margin_start(outer, 8);
    gtk_widget_set_margin_end(outer, 8);

    GtkWidget *thumb = gtk_image_new_from_file(r->path);
    gtk_widget_set_size_request(thumb, 96, 72);
    gtk_image_set_pixel_size(GTK_IMAGE(thumb), 96);
    gtk_box_append(GTK_BOX(outer), thumb);

    GtkWidget *mid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    char title[512];
    snprintf(title, sizeof(title), "#%d [%s] %.2f%%  hash %d/64",
        r->rank, r->label[0] ? r->label : "MATCH", r->similarity_percent, r->hash_distance);
    GtkWidget *title_label = gtk_label_new(title);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0f);
    gtk_box_append(GTK_BOX(mid), title_label);

    GtkWidget *path_label = gtk_label_new(r->path);
    gtk_label_set_xalign(GTK_LABEL(path_label), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(path_label), TRUE);
    gtk_box_append(GTK_BOX(mid), path_label);

    char meta[256];
    snprintf(meta, sizeof(meta), "%dx%d  %lld bytes", r->width, r->height, r->file_size_bytes);
    GtkWidget *meta_label = gtk_label_new(meta);
    gtk_label_set_xalign(GTK_LABEL(meta_label), 0.0f);
    gtk_box_append(GTK_BOX(mid), meta_label);

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    GtkWidget *reveal = gtk_button_new_with_label("Go to Directory");
    GtkWidget *open = gtk_button_new_with_label("Open");
    GtkWidget *copy = gtk_button_new_with_label("Copy Path");
    GtkWidget *similar = gtk_button_new_with_label("Search Similar");

    GtkWidget *arr[] = { reveal, open, copy, similar };
    for (int i = 0; i < 4; i++) {
        g_object_set_data_full(G_OBJECT(arr[i]), "loogal-path", g_strdup(r->path), g_free);
        g_object_set_data(G_OBJECT(arr[i]), "loogal-state", st);
        gtk_box_append(GTK_BOX(buttons), arr[i]);
    }

    g_signal_connect(reveal, "clicked", G_CALLBACK(on_reveal_clicked), NULL);
    g_signal_connect(open, "clicked", G_CALLBACK(on_open_clicked), NULL);
    g_signal_connect(copy, "clicked", G_CALLBACK(on_copy_clicked), NULL);
    g_signal_connect(similar, "clicked", G_CALLBACK(on_similar_clicked), NULL);

    gtk_box_append(GTK_BOX(mid), buttons);
    gtk_box_append(GTK_BOX(outer), mid);

    return outer;
}

static void load_page(AppState *st) {
    char cmd[4096];
    int n = snprintf(cmd, sizeof(cmd),
        "./loogal window-api page --session %s --offset %d --limit %d --json",
        st->session_id, st->offset, st->limit);
    if (n < 0 || n >= (int)sizeof(cmd)) {
        set_status(st, "window-api page command too long");
        return;
    }

    char *json = run_command_capture(cmd);
    if (!json) {
        set_status(st, "failed to load page");
        return;
    }

    LoogalWindowPage page;
    int ok = parse_page_json(json, &page);
    free(json);

    if (!ok) {
        set_status(st, "failed to parse page json");
        return;
    }

    clear_list(st->list_box);

    for (int i = 0; i < page.returned; i++) {
        GtkWidget *row = make_result_row(st, &page.results[i]);
        gtk_box_append(GTK_BOX(st->list_box), row);
    }

    char status[512];
    snprintf(status, sizeof(status),
        "Session: %s | offset %d | showing %d | total hits %d",
        st->session_id, st->offset, page.returned, page.total_hits);
    set_status(st, status);

    gtk_widget_set_sensitive(st->prev_button, st->offset > 0);
    gtk_widget_set_sensitive(st->next_button, page.returned >= st->limit);
    gtk_widget_set_sensitive(st->back_button, st->history_index > 0);
    gtk_widget_set_sensitive(st->forward_button, st->history_index >= 0 && st->history_index < st->history_count - 1);
}

static void on_next_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *st = user_data;
    st->offset += st->limit;
    load_page(st);
}

static void on_prev_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *st = user_data;
    st->offset -= st->limit;
    if (st->offset < 0) st->offset = 0;
    load_page(st);
}

static void on_back_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *st = user_data;
    if (st->history_index <= 0) return;
    st->history_index--;
    snprintf(st->session_id, sizeof(st->session_id), "%s", st->history[st->history_index]);
    st->offset = 0;
    load_page(st);
}

static void on_forward_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *st = user_data;
    if (st->history_index < 0 || st->history_index >= st->history_count - 1) return;
    st->history_index++;
    snprintf(st->session_id, sizeof(st->session_id), "%s", st->history[st->history_index]);
    st->offset = 0;
    load_page(st);
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppState *st = user_data;

    st->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(st->window), "Loogal Window");
    gtk_window_set_default_size(GTK_WINDOW(st->window), 980, 720);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(root, 10);
    gtk_widget_set_margin_bottom(root, 10);
    gtk_widget_set_margin_start(root, 10);
    gtk_widget_set_margin_end(root, 10);
    gtk_window_set_child(GTK_WINDOW(st->window), root);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    st->back_button = gtk_button_new_with_label("← Back");
    st->forward_button = gtk_button_new_with_label("Forward →");
    st->prev_button = gtk_button_new_with_label("Previous Page");
    st->next_button = gtk_button_new_with_label("Next Page");

    gtk_box_append(GTK_BOX(top), st->back_button);
    gtk_box_append(GTK_BOX(top), st->forward_button);
    gtk_box_append(GTK_BOX(top), st->prev_button);
    gtk_box_append(GTK_BOX(top), st->next_button);
    gtk_box_append(GTK_BOX(root), top);

    st->status_label = gtk_label_new("Loogal window loading...");
    gtk_label_set_xalign(GTK_LABEL(st->status_label), 0.0f);
    gtk_box_append(GTK_BOX(root), st->status_label);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    st->list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), st->list_box);
    gtk_box_append(GTK_BOX(root), scroll);

    g_signal_connect(st->next_button, "clicked", G_CALLBACK(on_next_clicked), st);
    g_signal_connect(st->prev_button, "clicked", G_CALLBACK(on_prev_clicked), st);
    g_signal_connect(st->back_button, "clicked", G_CALLBACK(on_back_clicked), st);
    g_signal_connect(st->forward_button, "clicked", G_CALLBACK(on_forward_clicked), st);

    gtk_window_present(GTK_WINDOW(st->window));
    load_page(st);
}

static void usage(FILE *f) {
    fprintf(f,
        "LOOGAL WINDOW\n"
        "\n"
        "Usage:\n"
        "  loogal-window --session <session_id>\n"
        "\n"
        "Example:\n"
        "  ./loogal-window --session session_123\n"
        "\n");
}

int main(int argc, char **argv) {
    AppState st;
    memset(&st, 0, sizeof(st));
    st.offset = 0;
    st.limit = LOOGAL_WINDOW_PAGE_LIMIT;
    st.history_index = -1;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--session") && i + 1 < argc) {
            snprintf(st.session_id, sizeof(st.session_id), "%s", argv[++i]);
        } else if (!strcmp(argv[i], "--offset") && i + 1 < argc) {
            st.offset = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--limit") && i + 1 < argc) {
            st.limit = atoi(argv[++i]);
            if (st.limit <= 0 || st.limit > LOOGAL_WINDOW_MAX_RESULTS) st.limit = LOOGAL_WINDOW_PAGE_LIMIT;
        } else if (!strcmp(argv[i], "help") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(stdout);
            return 0;
        }
    }

    if (!st.session_id[0]) {
        usage(stderr);
        fprintf(stderr, "[loogal-window:error] missing --session\n");
        return 1;
    }

    history_push(&st, st.session_id);

    GtkApplication *app = gtk_application_new("local.loogal.window", G_APPLICATION_DEFAULT_FLAGS);
    st.app = app;
    g_signal_connect(app, "activate", G_CALLBACK(activate), &st);
    /*
     * We parse Loogal-specific CLI flags ourselves above.
     * Do not pass them through to GTK/GApplication, or GTK will treat
     * --session/--offset/--limit as unknown options or file-open requests.
     */
    char *gtk_argv[] = { argv[0], NULL };
    int gtk_argc = 1;
    int status = g_application_run(G_APPLICATION(app), gtk_argc, gtk_argv);
    g_object_unref(app);
    return status;
}
