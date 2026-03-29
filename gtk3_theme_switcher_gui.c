#include <gtk/gtk.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

// ─────────────────────────────────────────────
// Check if directory contains gtk-3.0/ (valid GTK3 theme)
// ─────────────────────────────────────────────
static int is_gtk3_theme(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/gtk-3.0", dir);

    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

// ─────────────────────────────────────────────
// Scan themes directory
// ─────────────────────────────────────────────
static void scan_theme_dir(const char *base, GtkComboBoxText *combo) {
    DIR *d = opendir(base);
    if (!d) return;

    struct dirent *de;
    while ((de = readdir(d))) {
        if (de->d_name[0] == '.') continue;

        char full[512];
        snprintf(full, sizeof(full), "%s/%s", base, de->d_name);

        if (is_gtk3_theme(full)) {
            gtk_combo_box_text_append_text(combo, de->d_name);
        }
    }
    closedir(d);
}

// ─────────────────────────────────────────────
// Apply GTK theme
// ─────────────────────────────────────────────
static void apply_gtk_theme_c(const char *theme) {
    const char *home = getenv("HOME");
    if (!home) return;

    // Ensure directory exists
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/gtk-3.0", home);
    g_mkdir_with_parents(dir, 0755);

    // Write ~/.config/gtk-3.0/settings.ini
    char path[512];
    snprintf(path, sizeof(path), "%s/settings.ini", dir);

    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f,
            "[Settings]\n"
            "gtk-theme-name=%s\n",
            theme
        );
        fclose(f);
    }

    // GNOME / GTK4 system theme change
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "gsettings set org.gnome.desktop.interface gtk-theme \"%s\"",
        theme
    );
    system(cmd);
}

// ─────────────────────────────────────────────
// GTK Button callback
// ─────────────────────────────────────────────
static void on_apply_clicked(GtkButton *btn, gpointer user_data) {
    GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(user_data);
    gchar *theme = gtk_combo_box_text_get_active_text(combo);

    if (theme) {
        apply_gtk_theme_c(theme);
        g_print("Applied theme: %s\n", theme);

        GtkWidget *dialog = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Theme applied: %s", theme
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        g_free(theme);
    }
}

// ─────────────────────────────────────────────
// MAIN GUI APPLICATION
// ─────────────────────────────────────────────
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Main window
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "GTK3 Theme Switcher");
    gtk_window_set_default_size(GTK_WINDOW(win), 420, 200);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Layout
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(box), 16);
    gtk_container_add(GTK_CONTAINER(win), box);

    // Theme dropdown
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 0);

    // Scan directories
    scan_theme_dir("/usr/share/themes", GTK_COMBO_BOX_TEXT(combo));

    char home_path[512];
    snprintf(home_path, sizeof(home_path), "%s/.themes", getenv("HOME"));
    scan_theme_dir(home_path, GTK_COMBO_BOX_TEXT(combo));

    // Apply button
    GtkWidget *btn = gtk_button_new_with_label("Apply Theme");
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_apply_clicked), combo);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
