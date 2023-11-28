#include <gtk/gtk.h>
#include <unistd.h>

static GtkWidget *label;

static void update_directory(GtkEntry *entry, gpointer user_data) {
    const char *directory = gtk_entry_get_text(entry);

    if (chdir(directory) != 0) {
        perror("chdir");
        // Handle error: display an error message, etc.
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            gtk_label_set_text(GTK_LABEL(label), cwd);
        } else {
            perror("getcwd");
            // Handle error: display an error message, etc.
        }
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Directory Changer");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(update_directory), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

