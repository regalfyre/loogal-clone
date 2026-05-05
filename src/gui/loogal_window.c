
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *root;
    GtkWidget *flow;
    GtkWidget *status;
    GPtrArray *paths;
    char session_id[256];
    char place[4096];
    int current_index;
    int zoom_px;
} App;

static GtkWidget *viewer_win = NULL;
static GtkWidget *viewer_img = NULL;

static void set_status(App *app, const char *msg) {
    if (app && app->status && msg) gtk_label_set_text(GTK_LABEL(app->status), msg);
    if (msg) printf("%s\n", msg);
}

static int is_image_path(const char *p) {
    const char *dot = p ? strrchr(p, '.') : NULL;
    if (!dot) return 0;
    return !g_ascii_strcasecmp(dot, ".jpg") ||
           !g_ascii_strcasecmp(dot, ".jpeg") ||
           !g_ascii_strcasecmp(dot, ".png") ||
           !g_ascii_strcasecmp(dot, ".webp");
}

static int json_get_string_after(const char *start, const char *key, char *out, size_t out_sz) {
    const char *p = strstr(start, key);
    if (!p) return 0;
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    while (*p && g_ascii_isspace(*p)) p++;
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

static void clear_paths(App *app) {
    if (!app->paths) app->paths = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_set_size(app->paths, 0);
}

static void add_path(App *app, const char *path) {
    if (path && path[0]) g_ptr_array_add(app->paths, g_strdup(path));
}

static void load_paths_from_session(App *app) {
    clear_paths(app);

    if (!app->session_id[0]) return;

    char *argv_api[] = {
        "./loogal",
        "window-api",
        "page",
        "--session",
        app->session_id,
        "--offset",
        "0",
        "--limit",
        "256",
        "--json",
        NULL
    };

    char *stdout_buf = NULL;
    char *stderr_buf = NULL;
    int status = 0;
    GError *err = NULL;

    gboolean ok = g_spawn_sync(
        NULL,
        argv_api,
        NULL,
        G_SPAWN_SEARCH_PATH,
        NULL,
        NULL,
        &stdout_buf,
        &stderr_buf,
        &status,
        &err
    );

    if (!ok || !stdout_buf) {
        if (err) {
            set_status(app, err->message);
            g_error_free(err);
        } else {
            set_status(app, "failed to load session");
        }
        g_free(stdout_buf);
        g_free(stderr_buf);
        return;
    }

    const char *p = stdout_buf;
    while ((p = strstr(p, "\"path\""))) {
        char path[4096] = {0};
        if (json_get_string_after(p, "\"path\"", path, sizeof(path))) {
            add_path(app, path);
        }
        p += 6;
    }

    g_free(stdout_buf);
    g_free(stderr_buf);
}

static void on_viewer_destroy(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    viewer_win = NULL;
    viewer_img = NULL;
}

static void show_viewer_image(App *app) {
    if (!app || !app->paths || app->paths->len == 0 || !viewer_img) return;

    if (app->current_index < 0) app->current_index = 0;
    if ((guint)app->current_index >= app->paths->len)
        app->current_index = (int)app->paths->len - 1;

    const char *path = g_ptr_array_index(app->paths, app->current_index);
    gtk_image_set_from_file(GTK_IMAGE(viewer_img), path);
    gtk_image_set_pixel_size(GTK_IMAGE(viewer_img), app->zoom_px);

    char title[1200];
    snprintf(title, sizeof(title), "Loogal Viewer — %d / %u — zoom %dpx — %s",
             app->current_index + 1, app->paths->len, app->zoom_px, path);
    gtk_window_set_title(GTK_WINDOW(viewer_win), title);
}

static gboolean on_viewer_key(GtkEventControllerKey *controller,
                              guint keyval,
                              guint keycode,
                              GdkModifierType state,
                              gpointer data) {
    (void)controller;
    (void)keycode;
    (void)state;

    App *app = data;
    if (!app || !app->paths || app->paths->len == 0) return FALSE;

    if (keyval == GDK_KEY_Right) {
        app->current_index = (app->current_index + 1) % (int)app->paths->len;
        show_viewer_image(app);
        return TRUE;
    }

    if (keyval == GDK_KEY_Left) {
        app->current_index--;
        if (app->current_index < 0) app->current_index = (int)app->paths->len - 1;
        show_viewer_image(app);
        return TRUE;
    }

    if (keyval == GDK_KEY_plus || keyval == GDK_KEY_equal || keyval == GDK_KEY_KP_Add) {
        app->zoom_px += 120;
        if (app->zoom_px > 2400) app->zoom_px = 2400;
        show_viewer_image(app);
        return TRUE;
    }

    if (keyval == GDK_KEY_minus || keyval == GDK_KEY_KP_Subtract) {
        app->zoom_px -= 120;
        if (app->zoom_px < 120) app->zoom_px = 120;
        show_viewer_image(app);
        return TRUE;
    }

    if (keyval == GDK_KEY_0 || keyval == GDK_KEY_KP_0) {
        app->zoom_px = 720;
        show_viewer_image(app);
        return TRUE;
    }

    if (keyval == GDK_KEY_Escape) {
        gtk_window_close(GTK_WINDOW(viewer_win));
        return TRUE;
    }

    return FALSE;
}

static void add_drop_target(GtkWidget *widget, App *app);

static void open_viewer(App *app, int index) {
    if (!app || !app->paths || app->paths->len == 0) return;
    if (index < 0 || (guint)index >= app->paths->len) return;

    app->current_index = index;
    if (app->zoom_px <= 0) app->zoom_px = 720;

    if (!viewer_win) {
        viewer_win = gtk_window_new();
        gtk_window_set_default_size(GTK_WINDOW(viewer_win), 1000, 760);
        add_drop_target(viewer_win, app);

        GtkWidget *scroll = gtk_scrolled_window_new();
        gtk_window_set_child(GTK_WINDOW(viewer_win), scroll);

        viewer_img = gtk_image_new();
        gtk_widget_set_hexpand(viewer_img, TRUE);
        gtk_widget_set_vexpand(viewer_img, TRUE);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), viewer_img);

        GtkEventController *keys = gtk_event_controller_key_new();
        gtk_widget_add_controller(viewer_win, keys);
        g_signal_connect(keys, "key-pressed", G_CALLBACK(on_viewer_key), app);

        g_signal_connect(viewer_win, "destroy", G_CALLBACK(on_viewer_destroy), NULL);
    }

    show_viewer_image(app);
    gtk_window_present(GTK_WINDOW(viewer_win));
}

static gboolean on_main_key(GtkEventControllerKey *controller,
                            guint keyval,
                            guint keycode,
                            GdkModifierType state,
                            gpointer data) {
    (void)controller;
    (void)keycode;
    (void)state;

    App *app = data;

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        open_viewer(app, app->current_index);
        return TRUE;
    }

    if (keyval == GDK_KEY_Right) {
        if (app->paths && app->paths->len)
            app->current_index = (app->current_index + 1) % (int)app->paths->len;
        return TRUE;
    }

    if (keyval == GDK_KEY_Left) {
        if (app->paths && app->paths->len) {
            app->current_index--;
            if (app->current_index < 0) app->current_index = (int)app->paths->len - 1;
        }
        return TRUE;
    }

    return FALSE;
}

static void on_card_click(GtkGestureClick *gesture,
                          int n_press,
                          double x,
                          double y,
                          gpointer data) {
    (void)x;
    (void)y;

    App *app = data;
    GtkWidget *card = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    int index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(card), "loogal-index"));

    app->current_index = index;

    if (n_press >= 2) {
        open_viewer(app, index);
    }
}

static void add_image_card(App *app, const char *path, int idx) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(card, 8);
    gtk_widget_set_margin_bottom(card, 8);
    gtk_widget_set_margin_start(card, 8);
    gtk_widget_set_margin_end(card, 8);
    gtk_widget_set_size_request(card, 180, 170);

    GtkWidget *img = gtk_image_new_from_file(path);
    gtk_image_set_pixel_size(GTK_IMAGE(img), 150);
    gtk_widget_set_size_request(img, 160, 120);
    gtk_box_append(GTK_BOX(card), img);

    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;

    GtkWidget *label = gtk_label_new(base);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 20);
    gtk_box_append(GTK_BOX(card), label);

    g_object_set_data(G_OBJECT(card), "loogal-index", GINT_TO_POINTER(idx));

    GtkGesture *click = gtk_gesture_click_new();
    gtk_widget_add_controller(card, GTK_EVENT_CONTROLLER(click));
    g_signal_connect(click, "pressed", G_CALLBACK(on_card_click), app);

    gtk_flow_box_insert(GTK_FLOW_BOX(app->flow), card, -1);
}

static void populate_grid(App *app) {
    if (!app->flow) return;

    for (guint i = 0; i < app->paths->len; i++) {
        add_image_card(app, g_ptr_array_index(app->paths, i), (int)i);
    }

    char msg[512];
    snprintf(msg, sizeof(msg),
             "Session %s — %u results — drop an image anywhere to open a new search window",
             app->session_id, app->paths->len);
    set_status(app, msg);
}

static int create_session_for_drop(App *app, const char *path, char *session_id, size_t session_sz) {
    char *argv_session[] = {
        "./loogal",
        "session",
        "create",
        "--query",
        (char *)path,
        "--place",
        app->place,
        "--json",
        NULL
    };

    char *stdout_buf = NULL;
    char *stderr_buf = NULL;
    int status = 0;
    GError *err = NULL;

    gboolean ok = g_spawn_sync(
        NULL,
        argv_session,
        NULL,
        G_SPAWN_SEARCH_PATH,
        NULL,
        NULL,
        &stdout_buf,
        &stderr_buf,
        &status,
        &err
    );

    if (!ok || !stdout_buf) {
        if (err) {
            set_status(app, err->message);
            g_error_free(err);
        } else {
            set_status(app, "session create failed");
        }
        g_free(stdout_buf);
        g_free(stderr_buf);
        return 0;
    }

    int found = json_get_string_after(stdout_buf, "\"id\"", session_id, session_sz);

    if (!found) {
        char msg[512];
        snprintf(msg, sizeof(msg), "id not found. stdout: %.300s", stdout_buf);
        set_status(app, msg);
    }

    g_free(stdout_buf);
    g_free(stderr_buf);
    return found;
}

static void spawn_window_for_session(App *app, const char *session_id) {
    char *argv_window[] = {
        "./loogal-window",
        "--session",
        (char *)session_id,
        "--place",
        app->place,
        NULL
    };

    GError *err = NULL;
    gboolean ok = g_spawn_async(
        NULL,
        argv_window,
        NULL,
        G_SPAWN_SEARCH_PATH,
        NULL,
        NULL,
        NULL,
        &err
    );

    if (!ok) {
        if (err) {
            set_status(app, err->message);
            g_error_free(err);
        } else {
            set_status(app, "failed to open new window");
        }
        return;
    }

    char msg[512];
    snprintf(msg, sizeof(msg), "opened new Loogal search window: %s", session_id);
    set_status(app, msg);
}

static gboolean on_drop_files(GtkDropTarget *target,
                              const GValue *value,
                              double x,
                              double y,
                              gpointer data) {
    (void)target;
    (void)x;
    (void)y;

    App *app = data;
    set_status(app, "drop event received");

    GdkFileList *file_list = g_value_get_boxed(value);
    if (!file_list) {
        set_status(app, "drop failed: no file list");
        return FALSE;
    }

    GSList *files = gdk_file_list_get_files(file_list);
    if (!files) {
        set_status(app, "drop failed: empty file list");
        return FALSE;
    }

    GFile *file = G_FILE(files->data);
    char *path = g_file_get_path(file);
    if (!path) {
        set_status(app, "drop failed: could not get local path");
        return FALSE;
    }

    if (!is_image_path(path)) {
        set_status(app, "drop ignored: not a supported image");
        g_free(path);
        return FALSE;
    }

    char msg[4096];
    snprintf(msg, sizeof(msg), "drop accepted: %s", path);
    set_status(app, msg);

    char new_session[256] = {0};
    if (create_session_for_drop(app, path, new_session, sizeof(new_session))) {
        spawn_window_for_session(app, new_session);
    }

    g_free(path);
    return TRUE;
}

static void add_drop_target(GtkWidget *widget, App *app) {
    GtkDropTarget *drop = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
    g_signal_connect(drop, "drop", G_CALLBACK(on_drop_files), app);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(drop));
}

static void build_drop_landing(App *app) {
    GtkWidget *title = gtk_label_new("LOOGAL");
    gtk_widget_add_css_class(title, "title-1");
    gtk_box_append(GTK_BOX(app->root), title);

    GtkWidget *label = gtk_label_new("Drop an image here");
    gtk_widget_add_css_class(label, "title-3");
    gtk_box_append(GTK_BOX(app->root), label);

    app->status = gtk_label_new("waiting for image drop...");
    gtk_label_set_wrap(GTK_LABEL(app->status), TRUE);
    gtk_box_append(GTK_BOX(app->root), app->status);
}

static void build_session_grid(App *app) {
    GtkWidget *brand = gtk_label_new("LOOGAL — local visual memory");
    gtk_label_set_xalign(GTK_LABEL(brand), 0.0f);
    gtk_box_append(GTK_BOX(app->root), brand);

    app->status = gtk_label_new("loading session...");
    gtk_label_set_xalign(GTK_LABEL(app->status), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(app->status), TRUE);
    gtk_box_append(GTK_BOX(app->root), app->status);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(app->root), scroll);

    app->flow = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(app->flow), 6);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(app->flow), 2);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(app->flow), GTK_SELECTION_SINGLE);
    gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(app->flow), FALSE);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(app->flow), 10);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(app->flow), 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->flow);

    load_paths_from_session(app);
    populate_grid(app);
}

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    App *app = user_data;

    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "Loogal");
    gtk_window_set_default_size(GTK_WINDOW(app->window), app->session_id[0] ? 1100 : 760, app->session_id[0] ? 760 : 420);
    add_drop_target(app->window, app);

    GtkEventController *keys = gtk_event_controller_key_new();
    gtk_widget_add_controller(app->window, keys);
    g_signal_connect(keys, "key-pressed", G_CALLBACK(on_main_key), app);

    app->root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_top(app->root, 24);
    gtk_widget_set_margin_bottom(app->root, 24);
    gtk_widget_set_margin_start(app->root, 24);
    gtk_widget_set_margin_end(app->root, 24);
    gtk_window_set_child(GTK_WINDOW(app->window), app->root);
    add_drop_target(app->root, app);

    if (app->session_id[0]) build_session_grid(app);
    else build_drop_landing(app);

    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char **argv) {
    App app;
    memset(&app, 0, sizeof(app));

    app.paths = g_ptr_array_new_with_free_func(g_free);
    app.zoom_px = 720;

    const char *home = getenv("HOME");
    if (home) snprintf(app.place, sizeof(app.place), "%s/Pictures", home);
    else snprintf(app.place, sizeof(app.place), ".");

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--session") && i + 1 < argc) {
            snprintf(app.session_id, sizeof(app.session_id), "%s", argv[++i]);
        } else if (!strcmp(argv[i], "--place") && i + 1 < argc) {
            snprintf(app.place, sizeof(app.place), "%s", argv[++i]);
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            puts("LOOGAL WINDOW");
            puts("Usage:");
            puts("  ./loogal-window");
            puts("  ./loogal-window --session <session_id>");
            puts("  ./loogal-window --session <session_id> --place <search_root>");
            return 0;
        }
    }

    GtkApplication *gtk_app = gtk_application_new("local.loogal.drop-anywhere", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), &app);

    char *gtk_argv[] = { argv[0], NULL };
    int gtk_argc = 1;
    int status = g_application_run(G_APPLICATION(gtk_app), gtk_argc, gtk_argv);

    g_object_unref(gtk_app);
    g_ptr_array_free(app.paths, TRUE);

    return status;
}
