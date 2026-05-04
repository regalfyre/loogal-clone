
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *flow;
    GPtrArray *paths;
    int current_index;
    int zoom_px;
} App;

static GtkWidget *viewer_win = NULL;
static GtkWidget *viewer_img = NULL;

static void on_viewer_destroy(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data;
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
    (void)controller; (void)keycode; (void)state;
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

static void open_viewer(App *app, int index) {
    if (!app || !app->paths || app->paths->len == 0) return;
    if (index < 0 || (guint)index >= app->paths->len) return;

    app->current_index = index;
    if (app->zoom_px <= 0) app->zoom_px = 720;

    if (!viewer_win) {
        viewer_win = gtk_window_new();
        gtk_window_set_default_size(GTK_WINDOW(viewer_win), 1000, 760);

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
    (void)controller; (void)keycode; (void)state;
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
    (void)x; (void)y;
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

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    (void)user_data;

    App *app = calloc(1, sizeof(App));
    if (!app) return;

    app->paths = g_ptr_array_new_with_free_func(g_free);
    app->current_index = 0;
    app->zoom_px = 720;

    system("find \"$HOME/Pictures\" -type f \\( -iname '*.jpg' -o -iname '*.jpeg' -o -iname '*.png' -o -iname '*.webp' \\) 2>/dev/null | head -n 80 > /tmp/loogal_imgs.txt");

    FILE *f = fopen("/tmp/loogal_imgs.txt", "r");
    if (f) {
        char line[4096];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = 0;
            if (line[0]) g_ptr_array_add(app->paths, g_strdup(line));
        }
        fclose(f);
    }

    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "Loogal Grid — single click select, double click open, Enter open");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1000, 720);

    GtkEventController *keys = gtk_event_controller_key_new();
    gtk_widget_add_controller(app->window, keys);
    g_signal_connect(keys, "key-pressed", G_CALLBACK(on_main_key), app);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_window_set_child(GTK_WINDOW(app->window), scroll);

    app->flow = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(app->flow), 6);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(app->flow), 2);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(app->flow), GTK_SELECTION_SINGLE);
    gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(app->flow), FALSE);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(app->flow), 10);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(app->flow), 10);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->flow);

    for (guint i = 0; i < app->paths->len; i++) {
        add_image_card(app, g_ptr_array_index(app->paths, i), (int)i);
    }

    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("local.loogal.grid", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
